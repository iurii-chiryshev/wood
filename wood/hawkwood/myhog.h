#ifndef MYHOG_H
#define MYHOG_H

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/gpu/gpu.hpp>
#include <opencv2/ml/ml.hpp>
#include <string>
#include <math.h>
#include <vector>

using namespace cv;

class HOG
{
public:
    /*флаги дл¤ нормировки*/
    enum
    {
        NORM_L1 = 1,
        NORM_L2 = 2,
        NORM_L2HYS = 3,
    };
    /*число уровней*/
    enum { DEFAULT_NLEVELS=64 };
    /*манагер пам¤ти дл¤ hog*/
    class MemoryManager
    {
    public:
        MemoryManager(HOG* hogPtr,cv::Mat& img,cv::Size& winStride);
        ~MemoryManager();
        /*пересчитьть указатели*/
        void recount(cv::Mat& newImg);
        int getXmapBytes(const cv::Size& imgSize) const;
        int getYmapBytes(const cv::Size& imgSize) const;
        int getIntegrBufBytes(const cv::Size& imgSize) const;
        int getGradsBytes(const cv::Size& imgSize) const;
        int getCellsBytes(const cv::Size& imgSize) const;
        int getBlocksBytes(const cv::Size& imgSize) const;
        int getWinsBytes(const cv::Size& imgSize) const;
        cv::Size getNCells(const cv::Size& imgSize) const;
        cv::Size getNBlocks(const cv::Size& imgSize) const;
        cv::Size getNWins(const cv::Size& imgSize) const;
        cv::Size getWinStride() const;
        Mat image;
        Mat grads;
        Mat cells;
        Mat blocks;
        Mat wins;
        int* xmap;
        int* ymap;
        float* integrBuf;
    private:
        HOG* _hogPtr;
        void* _memPtr;
        cv::Size _imgSize;
        cv::Size _winStride;
        MemoryManager();
        MemoryManager(const MemoryManager& mm);
    };
    //************************************
    // Method:    ctor
    // Parameter: cv::Size win_size - размер окна, пикс
    // Parameter: cv::Size block_size - размер блока, пикс
    // Parameter: cv::Size block_stride - шаг смещени¤ блока, пикс
    // Parameter: cv::Size cell_size - размер ¤чейки, пикс
    // Parameter: int nbins - число направлений градиента
    // Parameter: int norma - нормировка
    // Parameter: double threshold_L2hys - порог, если используетс¤ NORM_L2HYS нормировка
    // Parameter: bool use_gamma - используетс¤ гамма?
    // Parameter: double gamma - коэффициент гаммы
    // Parameter: int nlevels - число уровней при много маштабной
    //************************************
    HOG(cv::Size win_size= cv::Size(64, 64), cv::Size block_size=cv::Size(16, 16),
        cv::Size block_stride=cv::Size(8, 8), cv::Size cell_size=Size(8, 8),
        int nbins=9, int norma = HOG::NORM_L2, double threshold_L2hys=0.2);
    ~HOG();

    //************************************
    // Method:    getDescriptorSize
    // Returns:   int
    // длина дескриптора
    //************************************
    int getDescriptorSize() const;

    //************************************
    // Method:    getNBlocks
    // Returns:   cv::Size
    // количество блоков в окне
    //************************************
    cv::Size getNBlocks() const;

    //************************************
    // Method:    getNCells
    // Returns:   cv::Size
    // количество ¤чеек в блоке
    //************************************
    cv::Size getNCells() const;

    //************************************
    // Method:    computeOne
    // Parameter: Mat & win - входное изображение размером win_size и типом CV_8UC1
    // Parameter: float * descr - указатель на начало дескриптора длиной getDescriptorSize
    // расчитает дескриптор дл¤ окна
    //************************************
    void computeOne(Mat& img,float* descr);

    void compute(Mat& img,int n);

    void detectMultiScale(Mat& img,vector<Rect>& foundLocations,vector<float>& foundWeights);

    void loadRTrees(const string& filename, const string& name);

private:
    cv::Size _winSize;
    cv::Size _blockSize;
    cv::Size _blockStride;
    cv::Size _cellSize;
    /*число направлений дл¤ гистограммы*/
    int _nbins;
    /*как норимровть вектор*/
    int _norma;
    /*порог в случае L2hys нормы*/
    double _L2hysThreshold;
    /*таблица дл¤ направлений*/
    cv::Mat _lut_qangle;
    /*таблица дл¤ градиента */
    cv::Mat _lut_grad;
    /*random tree*/
    Ptr<CvRTrees> _rtrees;

    //************************************
    // Method:    initLUT
    // инициализаци¤ таблиц дл¤ быстрого расчета градиентов
    //************************************
    void initLUT();

    //************************************
    // Method:    computeGradient
    // Parameter: const Mat& image - входное изображение, серое
    // Parameter: Mat& gradient - градиент изображени¤, согласно nbins
    // расчет градиента изображени¤
    //************************************
    void computeGradient(Ptr<MemoryManager> memoryManager);

    //************************************
    // Method:    precompWins
    // Parameter: Mat& image
    // Parameter: Mat& blocks
    // Parameter: cv::Size& winStride
    // расчет гистотграммы всех окон
    //************************************
    void precompWins(Ptr<MemoryManager> memoryManager);

    HOG(const HOG& hog);
};


#endif // MYHOG_H
