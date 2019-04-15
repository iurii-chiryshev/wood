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
#include <opencv2/gpu/gpu.hpp>
#include <opencv2/ml/ml.hpp>
#include<myhog.h>

using namespace std;
using namespace cv;

/**
 * @brief findFilesByMask
 * @param path
 * @param file_mask
 * @param files
 * вытащить из папки списком имена файлов по маске
 */
void findFilesByMask(const string& path, const string& file_mask, vector<string> &files){
    WIN32_FIND_DATAA FindFileData;
    HANDLE hf;
    string fileName;
    fileName.append(path).append("\\").append(file_mask);
    hf=FindFirstFileA(fileName.c_str(), &FindFileData);
    if (hf!=INVALID_HANDLE_VALUE){
        files.clear();
        do{
            files.push_back(FindFileData.cFileName);
        }
        while (FindNextFileA(hf,&FindFileData)!=0);
        FindClose(hf);
    }
}

/**
 * @brief splitString
 * @param s[in] строка
 * @param delim[in] разделитель
 * @param elems[out] подстроки
 * Выташить подстроки, разделенные разделителем
 */
void splitString(const string &s, char delim, vector<string> &elems) {
    stringstream ss;
    ss.str(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
}

/**
 * @brief fetchRects
 * @param[in] txtFileName
 * @param[out] rects
 * Вытаскиваем прямоугольники из файла
 */
void fetchRects(const string& txtFileName,vector<cv::Rect>& rects){
    rects.clear();

    std::ifstream file(txtFileName.c_str());
    if (!file.is_open())
        return;
    while (!file.eof())
    {
        string str;
        std::getline(file, str);
        if (str.empty()) break;
        vector<string> elements;
        splitString(str, ';', elements);
        if(elements.size() != 4) continue;
        cv::Rect rect(  std::stoi(elements[0]),
                        std::stoi(elements[1]),
                        std::stoi(elements[2]),
                        std::stoi(elements[3]));
        rects.push_back(rect);
    }
    file.close();

}

/**
 * @brief fetchImgFileName
 * @param[in] txtFileName
 * @param[out] imgFileName
 * Вытащить имя файла изображения из имени текстового файла
 */
void fetchImgFileName(const string &txtFileName, const string &prefix, string &imgFileName){
    imgFileName.clear();
    std::size_t start_pos = txtFileName.find(prefix);
    if(start_pos == 0){
        imgFileName = txtFileName.substr(prefix.length(),txtFileName.length() - prefix.length() - 4 /*.txt*/);
    }
}

struct RectResizer{
    /**
    * @brief RectResizer
    * @param dp - коэффициент заполнения ( 1 - 100 % ) площадь
    */
   RectResizer(int dp = 100): _dp(dp){}
   /**
    * @brief operator cv::Rect
    * @param rect
    */
   cv::Rect operator ()(const cv::Rect &rect) const{
       if(_dp >= 100) return cv::Rect(rect);
       if(_dp <= 0) return cv::Rect();
       double s = 100.0 / _dp; // скважность 0 - 1
       double scale = sqrtf(s); // коэф. масштабирования по стороне
       int w = cvRound(rect.width * scale);
       int h = cvRound(rect.height * scale);
       int x = rect.x  - (w - rect.width) / 2;
       int y = rect.y  - (h - rect.height) / 2;
       return cv::Rect(x,y,w,h);
   }
private:
   int _dp;
};


/**
 * @brief fetchRects
 * @param path - папка
 * @param prefixs - префиксы файлов positive/negative
 * @param results - результаты
 */
void fetchRects(const string &path,const string &prefix,map<string,vector<cv::Rect>> &result,const RectResizer &resizer = RectResizer()){
    vector<string> txtFiles;
    findFilesByMask(path,prefix + "*.txt",txtFiles);
    int count = 0;
    cout << "Count: " << txtFiles.size() << endl;
    result.clear();
    for(int i = 0; i < txtFiles.size();i++){
        const string& txtFileName = txtFiles[i];
        cout << "\r" << txtFileName;
        // вытаскиваю прямоугольники
        vector<cv::Rect> rects,frects;
        fetchRects(path + "\\" + txtFileName,rects);
        //cout << "\t" << "Count:" << rects.size() << endl;
        // вытаскиваю подстроку с именем файла
        string imgFileName;
        fetchImgFileName(txtFileName,prefix,imgFileName);
        //cout << "\t" << "Image name:" << imgFileName << endl;
        cv::Mat mat = cv::imread(path + "\\" + imgFileName);
        if(!mat.data){
            cerr << "\t" << "Error, read file" << endl;
        }
        for(int j = 0; j < rects.size(); j++){
            cv::Rect rect = resizer(rects[j]);
            if(     !(rect.x >= 0 &&
                    rect.y >= 0 &&
                    rect.x + rect.width <= mat.cols &&
                    rect.y + rect.height <= mat.rows)) continue;
            frects.push_back(rect);
        }
        result.insert(std::pair<string,vector<Rect>>(path + "\\" + imgFileName,frects));
    }
    cout << endl;
}

int getMapCount(map<string,vector<cv::Rect>> &rectMap){
    int count = 0;
    std::map<string,vector<cv::Rect>>::iterator it;
    for (it=rectMap.begin(); it!=rectMap.end(); it++){
        count += (int)it->second.size();
    }
    return count;
}

void reduseRects(map<string,vector<cv::Rect>> &src,map<string,vector<cv::Rect>> &dst,int maxVal = 2000){
    dst.clear();
    int selfCount = getMapCount(src);
    int percent = maxVal * 100 /  selfCount;
    RNG rng(42);
    std::map<string,vector<cv::Rect>>::iterator it;
    for (it=src.begin(); it!=src.end(); it++){
        const string key = it->first;
        const vector<cv::Rect> &values = it->second;
        vector<cv::Rect> dst_values;
        for(int i = 0; i < values.size(); i++){
            if(rng.uniform(0,100) < percent){
                dst_values.push_back(values[i]);
            }
        }
        if(dst_values.size() > 0)
            dst.insert(std::pair<string,vector<cv::Rect>>(key,dst_values));
    }

}


void computeHog(int winSize,int nbin,map<string,vector<Rect>> &src, vector<vector<float>> &dst){
    dst.clear();
    cv::RNG rng(time(NULL));
    HOG hog(cv::Size(winSize, winSize),
                          cv::Size(winSize >> 2, winSize >> 2),
                          cv::Size(winSize >> 3, winSize >> 3),
                          cv::Size(winSize >> 3, winSize >> 3),
                          nbin);
    std::map<string,vector<cv::Rect>>::iterator it;
    for (it=src.begin(); it!=src.end(); it++){
        string key = it->first;
        const vector<cv::Rect> &values = it->second;
        Mat img = cv::imread(key,CV_LOAD_IMAGE_COLOR);
        cout << winSize << "\r winSize, " << nbin << " bins, " << "computeHog for " << key;
        for(int j = 0; j < values.size(); j++){
            const cv::Rect &rect = values[j];
            Mat roi_img = img(rect);

            Mat gray_img,scale_img;
            cvtColor(roi_img,gray_img,CV_BGR2GRAY);

            double scaleFactor = double(winSize + 2)/gray_img.cols;
            cv::resize(gray_img,scale_img,Size(),scaleFactor,scaleFactor);
            cv::Rect roi(1,1,winSize,winSize);

            dst.push_back(vector<float>(hog.getDescriptorSize(),0));
            hog.computeOne(scale_img(roi),dst[dst.size() - 1].data());


//            for(int i = -1; i <= 1; i++,samplePtr += step)
//            {
            cv::Mat flip_img;
            cv::flip(scale_img,flip_img,rng.uniform(-1,2));
            /*применить*/
            dst.push_back(vector<float>(hog.getDescriptorSize(),0));
            hog.computeOne(flip_img(roi),dst[dst.size() - 1].data());
//            }
        }
    }
    cout << endl;
}


void saveToCsv(const std::string& fileName, vector<vector<float>> &pos, vector<vector<float>> &neg) {
    std::fstream outputFile;
    outputFile.open(fileName, std::ios::out);
    // positive
    for (int i = 0; i<pos.size(); i++)
    {
        const vector<float> &vec = pos[i];
        // первый столбец в файле - это метки классов response
        outputFile << 1;
        // все остальное - это данные
        for (int j = 0; j<vec.size(); j++)
        {
            outputFile << "," << vec[j];
        }
        outputFile << endl;

    }
    // negative
    for (int i = 0; i<neg.size(); i++)
    {
        const vector<float> &vec = neg[i];
        // первый столбец в файле - это метки классов response
        outputFile << 0;
        // все остальное - это данные
        for (int j = 0; j<vec.size(); j++)
        {
            outputFile << "," << vec[j];
        }
        outputFile << endl;

    }


    outputFile.close();
}


int main(int argc, char *argv[])
{
//    float minScale = 1, maxScale = 4;
//    int step = 10;
//    float delta = (maxScale -  minScale) / (step - 1);
//    vector<int> ds; for(int i = 0; i < step; i++) {
//        float scale = minScale + i * delta;
//        scale = 1 / scale;
//        scale = scale * scale * 100;
//        ds.push_back((int)scale);
//        cout << (int)scale << endl;
//    }
//    const vector<int> winSizes = {48,32,64};
//    const vector<int> nbins = {8,9} ;
//    const string path = "C:\\Users\\iurii.chiryshev\\Desktop\\hawkwood_forest";
//    const vector<string> prefixes = {"woodlogs_","antiwoodlogs_"};
//    for(int i = 0; i < ds.size(); i++){
//        const int d = ds[i];
//        cout << "d koeff = " << d << endl;
//        map<string,vector<Rect>> tmp, positive, negative;
//        fetchRects(path,prefixes[0],tmp,RectResizer(d));
//        fetchRects(path,prefixes[1],negative);
//        reduseRects(tmp,positive,getMapCount(negative)); // приводим примерно
////        cout << "positive size = " << getMapCount(positive) << endl;
////        cout << "negative size = " << getMapCount(negative) << endl;
//        for(int j = 0; j < winSizes.size(); j++){
//            for(int k = 0; k < nbins.size(); k++){
//                vector<vector<float>> out_pos, out_neg;
//                computeHog(winSizes[j],nbins[k],positive,out_pos);
//                computeHog(winSizes[j],nbins[k],negative,out_neg);
//                string fname = "HOG" + std::to_string(winSizes[j]) + "-" + std::to_string(nbins[k]) + "-" + std::to_string(d) + ".csv";
//                cout << "save " << fname << endl;
//                saveToCsv(fname,out_pos,out_neg);
//            }
//        }
//    }

    string prefix = "woodlogs_";
    string PATH = "D:\\foto\\HAWKwood\\Single Image Benchmark\\S.1 and S.2 real";

    vector<string> txtFiles;
    findFilesByMask(PATH,prefix + "*.txt",txtFiles);
    int count = 0;
    cout << "Count: " << txtFiles.size() << endl;
    for(int i = 0; i < txtFiles.size();i++){
        const string& txtFileName = txtFiles[i];
        cout << txtFileName << endl;
        // вытаскиваю прямоугольники
        vector<cv::Rect> rects;
        fetchRects(PATH + "\\" + txtFileName,rects);
        cout << "\t" << "Count:" << rects.size() << endl;
        // вытаскиваю подстроку с именем файла
        string imgFileName;
        fetchImgFileName(txtFileName,prefix,imgFileName);
        cout << "\t" << "Image name:" << imgFileName << endl;
        cv::Mat mat = cv::imread(PATH + "\\" + imgFileName);
        if(!mat.data){
            cerr << "\t" << "Error, read file" << endl;
        }
        for(int j = 0; j < rects.size(); j++){
            const cv::Rect &rect = rects[j];
            if(     !(rect.x >= 0 &&
                    rect.y >= 0 &&
                    rect.x + rect.width <= mat.cols &&
                    rect.y + rect.height <= mat.rows)) continue;
            if(j % 10 == 0){
                cv::imwrite(prefix + std::to_string(count) + ".bmp",mat(rect));
            }
            count++;
        }
    }
    //cout << PREFIXS[1] << ", rect count = " << count << endl;

    return 0;
}
