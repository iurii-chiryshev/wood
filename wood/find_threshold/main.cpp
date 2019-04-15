#include <iostream>
#include<windows.h>
#include <conio.h>
#include <time.h>
#define _USE_MATH_DEFINES // for C++
#include <math.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/gpu/gpu.hpp>
#include <opencv2/ml/ml.hpp>

using namespace std;
using namespace cv;

const string truth_path = "D:\\Projects\\2016\\foto\\find_threshold\\ground_truth";
const string threshold_path = "D:\\Projects\\2016\\foto\\find_threshold\\threshold_results";
const string alpha_path = "D:\\Projects\\2016\\foto\\find_threshold\\alpha_results";
const string ext = ".png";

/**
 * @brief find_files
 * @param path
 * @param file_mask
 * @param files
 * вытащить из папки списком имена файлов по маске
 */
void find_files(const string& path, const string& file_mask, vector<string> &files){
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
 * @brief confusion_matrix
 * @param truth
 * @param src
 * @param tp
 * @param fn
 * @param fp
 * @param tn
 */
void confusion_matrix(const Mat &truth,
                   const Mat &src,
                   int &tp,
                   int &fn,
                   int &fp,
                   int &tn){
    //https://en.wikipedia.org/wiki/Sensitivity_and_specificity
    CV_Assert(src.cols == truth.cols &&
              src.rows == truth.rows &&
              src.type() == CV_8UC1 &&
              src.type() == truth.type());
    const int width = src.cols,
            height = src.rows,
            step = src.step;
    tp = fn = fp = tn = 0;
    for(int y = 0; y < height; y++){
        uchar* pt = truth.data + step * y;
        uchar* ps = src.data + step * y;
        for(int x = 0; x < width; x++){
            if(pt[x] > 0 && ps[x] > 0) tp++;
            else if(pt[x] > 0 && ps[x] == 0) fn++;
            else if(pt[x] == 0 && ps[x] == 0) tn++;
            else if(pt[x] == 0 && ps[x] > 0) fp++;
        }
    }
    CV_Assert( (tp + fn + tn + fp) == (width * height) );
}

/**
 * @brief measures
 * @param truth
 * @param src
 * @param precision
 * @param recall
 * @param specificity
 * @param f1
 * @param fb
 * @param b
 */
void measures(const Mat &truth,
                    const Mat &src,
                    double &precision,
                    double &recall,
                    double &specificity,
                    double &f1,
                    double &fb,
                    double b = 1){
    //https://en.wikipedia.org/wiki/Sensitivity_and_specificity
    int tp,fn,fp,tn;
    confusion_matrix(truth,src,tp,fn,fp,tn);
    precision = tp == 0 ? 0 : (double) (tp) / (tp + fp);
    recall = tp == 0 ? 0 : (double)(tp) / (tp + fn); // TPR, hit rate
    specificity = tn == 0 ? 0 : (double) (tn) / (tn + fp);
    f1 = (precision == 0 && recall == 0) ? 0 : 2 * precision * recall / (precision + recall);
    if(b == 1 || f1 == 0) {
        fb = f1;
    }else{
        b *= b; // нужен квадрат
        fb = (1 + b) * precision * recall / ( b * precision + recall);
    }

}

/**
 * @brief read
 * @param path
 * @param pattern
 * @param suffx
 * @param thres
 * @param extension
 * @return
 */
Mat read(const string &path,
         const string &pattern,
         const string &suffx,
         int thres,
         const string &extension){
    string file;
    file.append(path)
            .append("\\")
            .append(pattern)
            .append(suffx)
            .append(std::to_string(thres))
            .append(extension);
    return cv::imread(file);
}

/**
 * @brief read
 * @param path
 * @param pattern
 * @param suffx
 * @param extension
 * @return
 */
Mat read(const string &path,
         const string &pattern,
         const string &suffx,
         const string &extension){
    string file;
    file.append(path)
            .append("\\")
            .append(pattern)
            .append(suffx)
            .append(extension);
    return cv::imread(file);
}

/**
 * @brief threshold
 * @param path
 * @param suffix
 * @param thress
 */
void threshold(const string &path,const string &suffix,const vector<int> &thress){
    //вытащить все png файлы из папки
    vector<string> files;
    find_files(truth_path,"*.png",files);

    //колическтво изоражений с конкретным порогом (alpha или бинаризация)
    vector<int> counts(thress.size(),0);

    //результаты
    const int scores = 5; // (precision, recall, specificity, f1, fb);
    vector<vector<double>> stats;
    for(int i = 0; i < counts.size(); i++){
        //
        // mean + std
        stats.push_back(vector<double>(scores * 2, 0));
    }

    const double betta = 2; // параметр для F метрики

    for(int i = 0 ; i < files.size(); i++){
        const string &file = files[i];
        // убираем окончание
        string pattern = file.substr(0,file.size() - 9); //_mask.png = 9 chars

        // truth
        Mat img_truth = read(truth_path,pattern,"_mask",ext);
        if(img_truth.data == NULL) continue;
        Mat truth, gray_truth;
        // src в два раза меньше truth
        cv::resize(img_truth,truth,cv::Size(),0.5,0.5,INTER_NEAREST);
        if(truth.channels() == 3) cvtColor(truth, gray_truth, CV_BGR2GRAY);
        else gray_truth = truth;

        std::cout << pattern << endl;


        for(int i = 0; i < thress.size(); i++){
            //по всем порогам
            int t = thress[i];
            Mat src = read(path,pattern,suffix,t,ext), gray_src;
            if(src.data == NULL) continue;
            // к серому
            if(src.channels() == 3) cvtColor(src, gray_src, CV_BGR2GRAY);
            else gray_src = src;

            if(gray_truth.type() != gray_src.type() ||
               gray_truth.size != gray_src.size) continue;


            double precision,recall,specificity,f1,fb;
            measures(gray_truth,gray_src,precision,recall,specificity,f1, fb, betta);
            //std::cout << t << " " << precision << " " << recall << " " << specificity << " " << f1 << "\n";

            counts[i]++;
            vector<double> &stat = stats[i];
            stat[0] += precision;
            stat[1] += recall;
            stat[2] += specificity;
            stat[3] += f1;
            stat[4] += fb;
            stat[0 + scores] += precision * precision;
            stat[1 + scores] += recall * recall;
            stat[2 + scores] += specificity * specificity;
            stat[3 + scores] += f1 * f1;
            stat[4 + scores] += fb * fb;

        }

    }

    // заголовок для данных
    vector<string> head = {
        suffix + ";",

        "Precision;",
        "Recall;",
        "Specificity;",
        "F1;",
        "F" + std::to_string((int)betta) + ";",

        "STD Precision;",
        "STD Recall;",
        "STD Specificity;",
        "STD F1;",
        "STD F" + std::to_string((int)betta),
    };
    for(int i = 0 ; i < head.size(); i++) {
        std::cout << head[i];
    }
    std::cout << endl;

    // расчет mean + std и вывод
    for(int i = 0; i < stats.size(); i++){
        vector<double> &stat = stats[i];
        int count = counts[i];
        if(count == 0) continue;

        for(int j = 0; j < scores; j++){
            stat[j] /= count;
            stat[j + scores] /= count;
            stat[j + scores] = sqrt(stat[j + scores] - stat[j] * stat[j]);
        }

        std::cout << thress[i] << ";";
        for(int j = 0; j < scores * 2; j++){
            std::cout << stat[j];
            if (j < scores * 2 - 1) std::cout << ";";
        }
        std::cout << endl;
    }
}

/**
 * @brief main
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[])
{
//    // для порога бинаризации
//    vector<int> thress;
//    for(int i = -4; i <= 60; i+=2){
//        thress.push_back(i);
//    }
//    threshold(threshold_path,"_tm_",thress);

    // для порога alpha
    vector<int> thress;
    for(int i = 1; i <= 10; i++){
        thress.push_back(i);
    }
    threshold(alpha_path,"_alpha_",thress);


    while (!_kbhit())
    {
        cv::waitKey(10);
    }
    cv::destroyAllWindows();
    return 0;
}
