#include <iostream>
#include<windows.h>
#include <conio.h>
#include <time.h>
#include <fstream>
#define _USE_MATH_DEFINES // for C++
#include <math.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/gpu/gpu.hpp>
#include <opencv2/ml/ml.hpp>


using namespace std;
using namespace cv;


/**
 * @brief findTmplPlace
 * @param src
 * @param tmpl
 * Помиск положения шаблона на изображении
 */
cv::Rect findTmplPlace(const Mat &src, const Mat &tmpl){
    // Расчет кореляционной картинки
    Mat res;
    cv::matchTemplate(src, tmpl, res, CV_TM_SQDIFF);

    // поиск минимумов и максимумов в кореляционной картинке
    double    minval, maxval;
    cv::Point    minloc, maxloc;
    cv::minMaxLoc(res, &minval, &maxval, &minloc, &maxloc);

    return cv::Rect(minloc,cv::Point(minloc.x + tmpl.cols - 1,minloc.y + tmpl.rows - 1));
}

cv::Rect findTimberPlace(const Mat &src, const Mat &tmpl)
{
    cv::Rect tmplRect =  findTmplPlace(src,tmpl);
    const float sP =  1.f / 6.f, sS = 2.f / 3.f;
    return cv::Rect(cv::Point(tmplRect.x + tmplRect.width * sP, tmplRect.y + tmplRect.height * sP),
                    cv::Size(tmplRect.width * sS, tmplRect.height * sS));

}

void findFilesByMask(const string& path, const string& file_mask, vector<string> &files){
    WIN32_FIND_DATAA FindFileData;
    HANDLE hf;
    string fileName;
    files.clear();
    fileName.append(path).append("\\").append(file_mask);
    hf=FindFirstFileA(fileName.c_str(), &FindFileData);
    if (hf!=INVALID_HANDLE_VALUE){
        do{
            string val;
            files.push_back(val.append(path).append("\\").append(FindFileData.cFileName));
        }
        while (FindNextFileA(hf,&FindFileData)!=0);
        FindClose(hf);
    }
}

void findFolders(const string& path, vector<string> &subFolders){
    subFolders.clear();
    WIN32_FIND_DATAA fd;
    HANDLE hf;
    string fileName;
    fileName.append(path).append("\\").append("*.*");
    hf=FindFirstFileA(fileName.c_str(), &fd);
    if (hf!=INVALID_HANDLE_VALUE){
        do{
            string stdFileName(fd.cFileName);
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
                    if (stdFileName != ".")
                        if (stdFileName != ".."){
                            string val;
                            subFolders.push_back(val.append(path).append("\\").append(stdFileName));
                        }

        }
        while (FindNextFileA(hf,&fd)!=0);
        FindClose(hf);
    }
}

/**
 * @brief foundRects
 * @param srcName
 * @param templNames
 * @param rects
 * @param convertToTimber
 */
cv::Mat foundRects(const string & srcName, const vector<string> & templNames, vector<Rect> &rects, bool convertToTimber = true){
    rects.clear();
    // пытаемся открыть исходное изображение
    Mat src = cv::imread(srcName);
    if(!src.data) return src;
    Mat graySrc;
    cv::cvtColor(src,graySrc,CV_BGR2GRAY);
    cout << "src: " + srcName << endl;
    for(int i = 0; i < templNames.size();i++){
        const string & templName = templNames[i];
        if(templName == srcName) continue;
        Mat tmpl = cv::imread(templName);
        if(!tmpl.data) continue;
        Mat grayTmpl;
        cv::cvtColor(tmpl,grayTmpl,CV_BGR2GRAY);
        cv::Rect rect = convertToTimber == true ? findTimberPlace(graySrc,grayTmpl) : findTmplPlace(src,tmpl);
        if( rect.x >= 0 &&
            rect.y >= 0 &&
            rect.x + rect.width <= graySrc.cols &&
            rect.y + rect.height <= graySrc.rows){
            rects.push_back(rect);
            cout << "\t template: " + templName << ", rect: " << rect << endl;
        }
    }

//    //показать чего вытащили
//    Mat rectSrc, viewSrc;
//    src.copyTo(rectSrc);
//    for(int i = 0; i < rects.size(); i++){
//        const Rect &rect = rects[i];
//        cv::rectangle(rectSrc, rect, cv::Scalar(0, 0, 255), 3);
//    }
//    if (rectSrc.cols > 1024)
//    {
//        cv::resize(rectSrc, viewSrc, cv::Size(), 1024. / rectSrc.cols, 1024. / rectSrc.cols);
//    }
//    else
//    {
//        rectSrc.copyTo(viewSrc);
//    }
//    imshow("example", viewSrc);
//    cv::waitKey(1000);

    return src;
}

string getNextFileName(){
    static int count = 0;
    return "FOREST_IMG_" + std::to_string(++count);
}

void saveRects(const std::string &dstFolderName,const Mat &src,const vector<Rect> &rects,const string &prefix){
    if( !src.data || rects.empty()) return;
    //сохранить изображение
    string srcName(getNextFileName().append(".jpg"));
    const vector<int> params = {CV_IMWRITE_JPEG_QUALITY,95};
    cv::imwrite(dstFolderName + "\\" + srcName,src,params);

    // сохраняем прямоугольники в csv
    std::fstream outputFile;
    string txtName = dstFolderName + "\\" + prefix + srcName + ".txt";
    outputFile.open(txtName, std::ios::out);
    for (int i = 0; i<rects.size(); i++)
    {
        const Rect &rect = rects[i];
        outputFile << rect.x << ";"
                   << rect.y << ";"
                   << rect.width << ";"
                   << rect.height << endl;
    }
    outputFile.close();
}

/**
 * @brief convert
 * @param srcFolder
 * @param dstFolder
 * @param prefix
 */
void convert (const std::string &srcFolderName, const std::string &dstFolderName, const string& prefix,bool convertToTimber = true){

    //найти все папки исходной папке
    vector<std::string> subFolderNames;
    findFolders(srcFolderName,subFolderNames);

    for(int i = 0; i < subFolderNames.size(); i++){
        const string &folderName = subFolderNames[i];
        // найти файл source.bmp
        vector<string> imageNames;
        findFilesByMask(folderName,"source.bmp",imageNames);
        if(imageNames.size() != 1) continue;
        string srcName = imageNames[0];
        // найти все файлы изображкения шаблоны + src
        imageNames.clear();
        findFilesByMask(folderName,"*.bmp",imageNames);
        vector<Rect> rects;

        Mat src = foundRects(srcName,imageNames,rects,convertToTimber);
        saveRects(dstFolderName,src,rects,prefix);
    }

}











int main(int argc, char *argv[])
{

    convert("D:\\2014\\Forest\\forest_research\\Kvinta.Forest.TrainingSet\\bin\\Release\\negative",
            "D:\\2014\\Forest\\forest_research\\Kvinta.Forest.TrainingSet\\bin\\Release\\hawk2forest",
            "antiwoodlogs_",
            false);
    convert("D:\\2014\\Forest\\forest_research\\Kvinta.Forest.TrainingSet\\bin\\Release\\positive",
            "D:\\2014\\Forest\\forest_research\\Kvinta.Forest.TrainingSet\\bin\\Release\\hawk2forest",
            "woodlogs_",
            true);
    return 0;
}
