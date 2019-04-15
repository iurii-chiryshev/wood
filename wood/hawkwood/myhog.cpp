#include "myhog.h"

    using namespace cv;
    using namespace std;


    HOG::HOG(cv::Size win_size/*= cv::Size(64, 64)*/, cv::Size block_size/*=cv::Size(16, 16)*/,
        cv::Size block_stride/*=cv::Size(8, 8)*/, cv::Size cell_size/*=Size(8, 8)*/,
        int nbins/*=9*/, int norma /*= HOG::NORM_L2*/, double threshold_L2hys/*=0.2*/)
        :_winSize(win_size),_blockSize(block_size),_blockStride(block_stride),
        _cellSize(cell_size), _nbins(nbins), _norma(norma), _L2hysThreshold(threshold_L2hys),_rtrees(new CvRTrees())
    {
        initLUT();
    }

    HOG::~HOG()
    {
    }

    void HOG::initLUT()
    {
        int x,y,hidx;
        const int width = 256 * 2 - 1;
        float angle,mag;
        const float angleScale = (float)(_nbins/CV_PI);
        AutoBuffer<float> autoBuf(width*4);
        float* dx_data = autoBuf;
        float* dy_data = dx_data + width;
        float* mag_data = dx_data + width*2;
        float* angle_data = dx_data + width*3;
        Mat Dx(1, width, CV_32F, dx_data);
        Mat Dy(1, width, CV_32F, dy_data);
        Mat Mag(1, width, CV_32F, mag_data);
        Mat Angle(1, width, CV_32F, angle_data);
        /*инициализируем все возможные dx (-255 : 255)*/
        for(x = 0 ;x < width; x++)
        {
            dx_data[x] = (float)(x - 255);
        }

        /*инициализаци¤ таблицы направлений и амплитуд*/
        _lut_qangle = cv::Mat(width,width,CV_8UC2); // int, 2 канала
        Mat grad = cv::Mat(width,width,CV_32FC2); // float, 2 канала
        float maxVal = 0;
        for(y = 0; y < width; y++)
        {
            /*инициализируем dy*/
            float yVal = (float)(y-255);
            for(x = 0; x < width; x++)
            {
                dy_data[x] = yVal;
            }
            /*dx, dy -> амплитуду и угол (0:2PI)*/
            cartToPolar(Dx,Dy,Mag,Angle,false);
            uchar *qangle_data = (uchar *)_lut_qangle.ptr(y);
            float *grad_data = (float *)grad.ptr(y);
            for(x = 0; x < width; x++,qangle_data += 2,grad_data += 2)
            {
                mag = mag_data[x];
                angle = angle_data[x]*angleScale - 0.5f;
                hidx = cvFloor(angle);
                angle -= hidx;
                grad_data[0] = (1.0f - angle)*mag;
                grad_data[1] = angle*mag;
                maxVal = MAX(maxVal,MAX(grad_data[0],grad_data[1]));
                if( hidx < 0 )
                    hidx += _nbins;
                else if( hidx >= _nbins )
                    hidx -= _nbins;
                qangle_data[0] = (uchar)hidx;
                hidx++;
                hidx &= hidx < _nbins ? -1 : 0;
                qangle_data[1] = (uchar)hidx;
            }
        }
        grad.convertTo(_lut_grad,grad.type(),1./maxVal);
    }

    struct GradientInvoker: ParallelLoopBody
    {
        GradientInvoker(Mat& img,Mat& grad,int nbins,Mat& lut_qangle,Mat& lut_grad,int *xmap,int *ymap)
        {
            /*CV_Assert( nbins == grad.channels() && img.size() == grad.size() );*/
            _img = &img;
            _grad = &grad;
            _lut_grad = &lut_grad;
            _lut_qangle = &lut_qangle;
            _nbins = nbins;
            _xmap = xmap;
            _ymap = ymap;
        }
        void operator()(const Range& range) const
        {
            uchar *lut_qanglePtr, *imgData = _img->data, *gradData = _grad->data;
            float *lut_gradPtr;
            int x,y,dx,dy,/*q0,q1,*/xm,b;
            const int	width = _img->cols, // ширина img
                        imgStep = _img->step,
                        gwidth = _grad->cols,
                        gheight = _grad->rows/_nbins,
                        gstep = _grad->step,
                        y0 = range.start, // начало диапазона
                        y1 = range.end; // конец диапазона
            float* gradPtrs[UCHAR_MAX+1];
            for(y = y0; y < y1; y++ )
            {

                const uchar* imgPtr  = imgData + imgStep*_ymap[y];
                const uchar* prevPtr = imgData + imgStep*_ymap[y-1];
                const uchar* nextPtr = imgData + imgStep*_ymap[y+1];
                for(b = 0; b < _nbins;b++)
                {
                    gradPtrs[b] = (float*)(gradData + (b * gheight + y + 1) * gstep);
                    for(x = 0; x < gwidth;x++)
                    {
                        gradPtrs[b][x] = 0;
                    }
                    gradPtrs[b]++;
                }
                for(x = 0; x < width; x++)
                {
                    xm = _xmap[x];
                    dx = (int)imgPtr[_xmap[x+1]] - (int)imgPtr[_xmap[x-1]] + 255;
                    dy = (int)nextPtr[xm] - (int)prevPtr[xm] + 255;
                    lut_qanglePtr = (uchar*)_lut_qangle->ptr(dy,dx);
                    lut_gradPtr = (float*)_lut_grad->ptr(dy,dx);
                    gradPtrs[lut_qanglePtr[0]][x] = lut_gradPtr[0];
                    gradPtrs[lut_qanglePtr[1]][x] = lut_gradPtr[1];
                }
            }
        }
    private:
        Mat* _img;
        Mat* _grad;
        Mat* _lut_qangle;
        Mat* _lut_grad;
        int _nbins;
        int* _xmap;
        int* _ymap;
    };

    void HOG::computeGradient(Ptr<MemoryManager> memoryManager)
    {
        Mat img = memoryManager->image,
            grads = memoryManager->grads;
        const int	width = img.cols,
                    height = img.rows,
                    borderType = (int)BORDER_REFLECT_101;
        Size wholeSize;
        Point roiOfs;
        img.locateROI(wholeSize, roiOfs);
        int* xmap = memoryManager->xmap + 1;
        int* ymap = memoryManager->ymap + 1;
        for(int x = -1; x < width + 1; x++ )
            xmap[x] = borderInterpolate(x + roiOfs.x,wholeSize.width, borderType) - roiOfs.x;
        for(int y = -1; y < height + 1; y++ )
            ymap[y] = borderInterpolate(y + roiOfs.y,wholeSize.height, borderType) - roiOfs.y;

#ifdef _DEBUG
        GradientInvoker gc(img,grads,_nbins,_lut_qangle,_lut_grad,xmap,ymap);
        gc(Range(0, height));
#else
        parallel_for_(Range(0, height),GradientInvoker(img,grads,_nbins,_lut_qangle,_lut_grad,xmap,ymap)/*,_nbins*/);
#endif // _DEBUG
    }

    int HOG::getDescriptorSize() const
    {
        CV_Assert(_blockSize.width % _cellSize.width == 0 &&
            _blockSize.height % _cellSize.height == 0);
        CV_Assert((_winSize.width - _blockSize.width) % _blockStride.width == 0 &&
            (_winSize.height - _blockSize.height) % _blockStride.height == 0 );
        return _nbins*
            (_blockSize.width/_cellSize.width)*
            (_blockSize.height/_cellSize.height)*
            ((_winSize.width - _blockSize.width)/_blockStride.width + 1)*
            ((_winSize.height - _blockSize.height)/_blockStride.height + 1);
    }

    cv::Size HOG::getNBlocks() const
    {
        return Size((_winSize.width - _blockSize.width)/_blockStride.width + 1,
            (_winSize.height - _blockSize.height)/_blockStride.height + 1);
    }

    cv::Size HOG::getNCells() const
    {
        return Size(_blockSize.width/_cellSize.width, _blockSize.height/_cellSize.height);
    }

    static void _integral(float* buf,Mat& img,int nbins)
    {
        const int width = img.cols, height = img.rows;
        int x,y;
        float	*currentBuf = buf,
                *nextBuf = buf + width,
                *currentPtr = (float*)img.ptr(0),
                *tmp;
        for(x = 0; x < width + width; ++x)
        {
            /*обнулить буферы*/
            currentBuf[x] = 0;
        }
        for(x = 0; x < width; ++x)
        {
            /*обнулить первую строку*/
            currentPtr[x] = 0;
        }
        for(y = 1; y < height; ++y)
        {
            currentPtr = (float*)img.ptr(y);
            for(x = 1; x < width; ++x)
            {
                /*дл¤ следующей итерации*/
                nextBuf[x] = currentBuf[x] + currentPtr[x];
            }
            for(x = 1; x < width; ++x)
            {
                /*интегральна¤ сумма */
                currentBuf[x] += currentBuf[x-1];
            }
            for(x = 1; x < width; ++x)
            {
                /*интегральна¤ сумма */
                currentPtr[x] += currentPtr[x-1];
            }
            for(x = 1; x < width; ++x)
            {
                /*финальный расчет*/
                currentPtr[x] += currentBuf[x];
            }
            /*мен¤ем местами буферы дл¤ следующей итерации*/
            tmp = nextBuf;
            nextBuf = currentBuf;
            currentBuf = tmp;
        }

    }
    static void _precompCells(const Mat& integr,Mat& cells,const Size& cellSize,const Size& winStride)
    {
        const int
            cwidth = cells.cols,
            cheight = cells.rows,
            wsh = winStride.height,
            wsw = winStride.width,
            csh = cellSize.height,
            csw = cellSize.width;
        int x,y,cx,cy;
        float a,b,c,d;
        for(y = 0,cy = 0; y < cheight; y++,cy += wsh)
        {
            float* cellsPtr = (float*)cells.ptr(y);
            for(x = 0,cx = 0; x < cwidth; x++, cx += wsw)
            {
                a = integr.at<float>(cy,cx);
                b = integr.at<float>(cy,cx + csw);
                d = integr.at<float>(cy + csh,cx);
                c = integr.at<float>(cy + csh,cx + csw);
                cellsPtr[x] = c - b - d + a;
#ifdef _DEBUG
                if(cellsPtr[x] < -0.002f)
                {
                    cout << "summ less 0 val- "<< cellsPtr[x] << "\n a- " << a << "\n b- " << b << "\n c- " << c << "\n d- " << d << "\n";
                }
#endif // DEBUG
            }
        }
    }
    static void _precompBlocks(const Mat& cells,Mat& blocks,const Size& jumpCell,const Size& ncellsInBlock,/*int nbins,*/int bin)
    {
        const int bwidth = blocks.cols,
                bheight = blocks.rows,
                bchannels = blocks.channels(),
                jumpStep = cells.step*jumpCell.height/sizeof(float),
                /*jumpCell_height = jumpCell.height,*/
                jumpCell_width = jumpCell.width,
                ncells_height = ncellsInBlock.height,
                ncells_width = ncellsInBlock.width;
        int x,y,cx,cy;
        for(y = 0; y < bheight; y++)
        {
            float* blocksPtr = (float*)blocks.ptr(y) + bin*ncellsInBlock.area();
            float* cellsPtr = (float*)cells.ptr(y);
            for(x = 0; x < bwidth; x++, cellsPtr++,blocksPtr += bchannels)
            {
                float* cPtr = cellsPtr;
                float* bPtr = blocksPtr;
                int histId = 0;
                for(cy = 0; cy < ncells_height; cy++,cPtr += jumpStep)
                {
                    for(cx = 0; cx < ncells_width; cx++ )
                    {
                        bPtr[histId++] = cPtr[cx*jumpCell_width];
                    }
                }
            }
        }
    }
    static void _normBlocks(const Mat& blocks,Mat& normBlocks,const Size& jumpCell,const Size& ncellsInBlock)
    {

    }
    /*распараллеливание не по строкам, а по каналам*/
    struct BlocksInvoker: ParallelLoopBody
    {
        BlocksInvoker(Mat& grad,Mat& cells,Mat& blocks,const Size& cellSize,const Size& winStride,const Size& jumpCell,const Size& ncellsInBlock,float* integrBuf,int nbins)
        {
            _grad = &grad;
            _cells = &cells;
            _blocks = &blocks;
            _cellSize = cellSize;
            _winStride = winStride;
            _jumpCell = jumpCell;
            _ncellsInBlock = ncellsInBlock;
            _integrBuf = integrBuf;
            _nbins = nbins;
        }
        void operator()(const Range& range) const
        {
            const int
                gwidth = _grad->cols,
                gheight = _grad->rows/_nbins,
                gstep = _grad->step,
                gtype = _grad->type(),

                cwidth = _cells->cols,
                cheight = _cells->rows/_nbins,
                cstep = _cells->step,
                ctype = _cells->type(),

                bwidth = _blocks->cols,
                bheight = _blocks->rows,
                bstep = _blocks->step,
                btype = _blocks->type(),

                cn0 = range.start,
                cn1 = range.end;
            const uchar *gdata = _grad->data,
                        *cdata = _cells->data,
                        *bdata = _blocks->data;
            for(int cn = cn0; cn < cn1; cn++ )
            {
                Mat subGrads(gheight,gwidth,gtype,(void*)(gdata + cn*gheight*gstep));
                Mat subCells(cheight,cwidth,ctype,(void*)(cdata + cn*cheight*cstep));
                Mat subBlocks(bheight,bwidth,btype,(void*)bdata);
                /*из src сделать интегральное представление*/
                _integral(_integrBuf + cn*(gwidth + gwidth),subGrads,_nbins);
                _precompCells(subGrads,subCells,_cellSize,_winStride);
                _precompBlocks(subCells,subBlocks,_jumpCell,_ncellsInBlock,/*_nbins,*/cn);
            }
        }
    private:
        Mat* _grad;
        Mat* _cells;
        Mat* _blocks;
        Size _cellSize;
        Size _winStride;
        Size _jumpCell;
        Size _ncellsInBlock;
        float* _integrBuf;
        int _nbins;

    };

    struct NormBlocksInvoker: ParallelLoopBody
    {
        NormBlocksInvoker(Mat& blocks,int norma,double L2hysThres)
        {
            _blocks = &blocks;
            _norma = norma;
            _L2hysThres = L2hysThres;
        }
        void operator()(const Range& range) const
        {
            int x,y,z;
            const int	width = _blocks->cols,
                        height = _blocks->rows,
                        channels = _blocks->channels(),
                        step = _blocks->step, // строка в байтах,
                        y0 = range.start, // начало диапазона
                        y1 = range.end; // конец диапазона
            const uchar* blocksData = _blocks->data;
            float L2hysThres = (float)_L2hysThres;
            if(_norma == HOG::NORM_L1)
            {
                for(y = y0; y < y1; y++ )
                {
                    float* blockPtr = (float*)(blocksData + step*y);
                    for(x = 0; x < width; x++,blockPtr += channels)
                    {
                        float summ = 0;
                        /*ищем сумму */
                        for (z = 0; z < channels; z++)
                        {
                            if(blockPtr[z] < 0)
                            {
                                blockPtr[z] = 0;
                                continue;
                            }
                            summ += blockPtr[z];
                        }
                        float scale = 1.f/(summ + 1.0e-05F);
                        /*нормировка*/
                        for (z = 0; z < channels; z++)
                        {
                            blockPtr[z] *= scale;
                        }
                    }
                }
            }
            else if(_norma == HOG::NORM_L2 || _norma == HOG::NORM_L2HYS)
            {
                for(y = y0; y < y1; y++ )
                {
                    float* blockPtr = (float*)(blocksData + step*y);
                    for(x = 0; x < width; x++,blockPtr += channels)
                    {
                        /*ищем сумму квадратов */
                        float summ = 0;
                        for (z = 0; z < channels; z++)
                        {
                            if(blockPtr[z] < 0)
                            {
                                blockPtr[z] = 0;
                                continue;
                            }
                            summ += blockPtr[z]*blockPtr[z];
                        }

                        float scale = 1.f/(std::sqrt(summ) + 1.0e-05F);
                        if(_norma == HOG::NORM_L2HYS)
                        {
                            /*переномировка*/
                            summ = 0;
                            for(z = 0; z < channels; z++)
                            {
                                blockPtr[z] = std::min(blockPtr[z]*scale, L2hysThres);
                                summ += blockPtr[z]*blockPtr[z];
                            }
                            scale = 1.f/(std::sqrt(summ) + 1.0e-05F);
                        }
                        /*нормировка*/
                        for (z = 0; z < channels; z++)
                        {
                            blockPtr[z] *= scale;
                        }
                    }
                }
            }
            else
            {
                CV_Error(CV_StsBadArg,"Norma mast be L1, L2 ore L2Hys");
            }

        }
    private:
        Mat* _blocks;
        int _norma;
        double _L2hysThres;
    };

    struct WinsInvoker: ParallelLoopBody
    {
        WinsInvoker(Mat& blocks, Mat& wins,const Size& jumpBlocks,const Size& nblocksInWin)
        {
            _blocks = &blocks;
            _wins = &wins;
            _jumpBlocks = jumpBlocks;
            _nblocksInWin = nblocksInWin;
        }
        void operator ()(const Range& range) const
        {
            const int   wwidth = _wins->cols,
                        wheight = _wins->rows,
                        wstep = _wins->step,
                        nblocksInWin = _nblocksInWin.area(), //
                        bstep = _blocks->step,
                        bcn = _blocks->channels(), //
                        jumpStep = _blocks->step*_jumpBlocks.height/sizeof(float),
                        jumpBlocks_width = _jumpBlocks.width*_blocks->channels(),
                        y0 = range.start,
                        y1 = range.end;
            int x,y,bx,by;
            uchar *winData = _wins->data, *blockData = _blocks->data;
            for(y = y0; y< y1; y++)
            {
                float** winPtr = (float**)(void *)(winData + wstep*y);
                float*  blockPtr = (float*)(blockData + bstep*y);
                for(x = 0; x < wwidth; x++, winPtr += nblocksInWin,blockPtr += bcn)
                {
                    float** wPtr = winPtr;
                    float* bPtr = blockPtr;
                    int blockId = 0;
                    for(by = 0; by < _nblocksInWin.height; by++,bPtr += jumpStep)
                    {
                        for(bx = 0; bx < _nblocksInWin.width; bx++)
                        {
                            wPtr[blockId++] = bPtr + bx*jumpBlocks_width;
                        }
                    }
                }
            }
        }
    private:
        Mat* _blocks;
        Mat* _wins;
        Size _jumpBlocks;
        Size _nblocksInWin;
    };

    void HOG::precompWins(Ptr<MemoryManager> memoryManager)
    {
        Mat img  = memoryManager->image,
            grads = memoryManager->grads,
            cells = memoryManager->cells,
            blocks = memoryManager->blocks,
            wins = memoryManager->wins;
        const int width = img.cols, height = img.rows;
        cv::Size	winStride = memoryManager->getWinStride(),
                    imgSize = img.size();

        /*через сколько блоков, ¤чеек прыгать, чтобы получить свои*/
        cv::Size jump = Size(_blockStride.width/winStride.width,
                            _blockStride.height/winStride.height);

        //////////////////////////////////win data////////////////////////////////////////////////////
        cv::Size nwins = memoryManager->getNWins(imgSize);
        /*количество блоков в окне*/
        cv::Size nblocksInWin = getNBlocks();

        //////////////////////////////////block data//////////////////////////////////////////////////
        /*количество блоков на изображении*/
        cv::Size nblocks = memoryManager->getNBlocks(imgSize);
        /*количество ¤чеек внутри блока (Ќ≈ ¬Ќ”“–» »«ќЅ–ј∆≈Ќ»я!)*/
        cv::Size ncellsInBlock = getNCells();

        //////////////////////////////////cell data//////////////////////////////////////////////////
        /*количество ¤чеек на изображении*/
        cv::Size ncells = memoryManager->getNCells(imgSize);

        ////////////////////////////////////расчет//////////////////////////////////////////////////////////////
        computeGradient(memoryManager);
#ifdef _DEBUG

        BlocksInvoker cc(grads,cells,blocks,_cellSize,winStride,jump,ncellsInBlock,memoryManager->integrBuf,_nbins);
        cc(Range(0,_nbins));

        NormBlocksInvoker ncb(blocks,_norma,_L2hysThreshold);
        ncb(Range(0,nblocks.height));

        WinsInvoker wi(blocks,wins,jump,nblocksInWin);
        wi(Range(0,nwins.height));

        cout << "cache stride- " << winStride.height << " x " << winStride.width << " pix\n\n";

        cout << "cell size- " << _cellSize.height << " x " << _cellSize.width << " pix\n";
        cout << "cells in blocks- " << ncellsInBlock.height << " x " << ncellsInBlock.width << "\n";
        cout << "cells in image- " << ncells.height << " x " << ncells.width << "\n\n";

        cout << "block size " << _blockSize.height << " x " << _blockSize.width << " pix\n";
        cout << "blocks in window " << nblocksInWin.height << " x " << nblocksInWin.width << "\n";
        cout << "blocks in image " << nblocks.height << " x " << nblocks.width << "\n\n";

        double maxVal, minVal;
        cv::minMaxIdx(grads,&minVal,&maxVal);
        cout << "intgral min- " << minVal << " max- " << maxVal << "\n";
        cv::minMaxIdx(cells,&minVal,&maxVal);
        cout << "cells min- " << minVal << " max- " << maxVal << "\n";
        cv::minMaxIdx(blocks,&minVal,&maxVal);
        cout << "blocks min- " << minVal << " max- " << maxVal << "\n\n";
#else
        parallel_for_(Range(0,_nbins),BlocksInvoker(grads,cells,blocks,_cellSize,winStride,jump,ncellsInBlock,memoryManager->integrBuf,_nbins));

        parallel_for_(Range(0,nblocks.height),NormBlocksInvoker(blocks,_norma,_L2hysThreshold));

        parallel_for_(Range(0,nwins.height),WinsInvoker(blocks,wins,jump,nblocksInWin));
#endif // _DEBUG




    }

    void HOG::computeOne(Mat& img,float* descr)
    {
        const int nblocks = getNBlocks().area(); // количество блоков
        const int nblockDesc = getDescriptorSize()/nblocks; // длина вектора в кажом блоке
        int i,j,n = 0;
        CV_Assert(img.data && (img.type() == CV_8UC1) && (img.size() == _winSize));

        Ptr<HOG::MemoryManager> memoryManager( new MemoryManager( this, img, Size() ) );
        precompWins(memoryManager);
        float** ppBlocks = (float**)(void*)(memoryManager->wins.data);
        AutoBuffer<float> buf(nblocks*nblockDesc);

        for(i = 0; i < nblocks; i++)
        {
            for(j = 0; j < nblockDesc; j++)
            {
                descr[n++] = ppBlocks[i][j];
            }
        }
    }

    void HOG::compute(Mat& img,int n)
    {
        CV_Assert(img.data && (img.type() == CV_8UC1));
        Ptr<HOG::MemoryManager> memoryManager( new MemoryManager( this, img, Size() ) );
        for(int i = 0; i< n; i++)
        {
            memoryManager->recount(img);
            precompWins(memoryManager);
        }
    }

    struct RTreesInvoker: ParallelLoopBody
    {
        RTreesInvoker(Mat& wins, int nblocks,int nblockDesc, const Size& winSize, const Size& winStride, double invScale,
                    CvRTrees* rtrees,vector<bool>* findRect,vector<cv::Rect>* rects,vector<float>* tempWeights)
        {
            _wins = &wins;
            _nblocks = nblocks;
            _nblockDesc = nblockDesc;
            _winSize = winSize;
            _winStride = winStride;
            _invScale = invScale;
            _rtrees = rtrees;
            _tempWeights = tempWeights;
            _rects = rects;
            _findRect = findRect;
        }
        void operator()(const Range& range) const
        {
            const int   w = _wins->cols,
                        y0 = range.start,
                        y1 = range.end;
            int i,j,y,x,n,r;
            Mat descr(1,_nblocks*_nblockDesc,CV_32FC1);
            float* descrPtr = (float*)(descr.data);
            for(y = y0; y < y1; ++y)
            {
                for(x = 0; x < w;++x)
                {
                    n = 0;
                    r = y*w + x;
                    float** ppBlocks = (float**)_wins->ptr(y,x);
                    for( i = 0; i < _nblocks; i++)
                    {
                        for(j = 0; j < _nblockDesc; j++)
                        {
                            descrPtr[n++] = ppBlocks[i][j];
                        }
                    }
                    if((int)_rtrees->predict(descr) == 1)
                    {
                        /*если классифицировал как бревно*/
                        _findRect->at(r) = true;
                        _tempWeights->at(r) = _rtrees->predict_prob(descr);
                        _rects->at(r) = cv::Rect(cvRound(x*_winStride.width*_invScale),
                            cvRound(y*_winStride.height*_invScale),
                            cvRound(_winSize.width*_invScale),
                            cvRound(_winSize.height*_invScale));
                    }
                    else
                    {
                        _findRect->at(r) = false;
                    }
                }
            }
        }
    private:
        Mat* _wins;
        int _nblocks; // количество блоков в окне
        int _nblockDesc; // длина вектора признаков в блоке
        Size _winSize; // размер окна
        Size _winStride; // отступ окна
        double _invScale; // масштабный коэффициент
        CvRTrees* _rtrees;
        vector<bool>* _findRect;
        vector<float>* _tempWeights;
        vector<cv::Rect>* _rects;
    };

    static void _groupRectangles(vector<cv::Rect>& rectList, vector<float>& weights, int groupThreshold, double eps)
    {
        if( groupThreshold <= 0 || rectList.empty() )
        {
            return;
        }

        CV_Assert(rectList.size() == weights.size());

        vector<int> labels;
        int nclasses = partition(rectList, labels, SimilarRects(eps));

        vector<cv::Rect_<float> > rrects(nclasses);
        vector<int> numInClass(nclasses, 0);
        vector<float> foundWeights(nclasses, FLT_MIN);
        int i, j, nlabels = (int)labels.size();

        for( i = 0; i < nlabels; i++ )
        {
            int cls = labels[i];
            rrects[cls].x += rectList[i].x;
            rrects[cls].y += rectList[i].y;
            rrects[cls].width += rectList[i].width;
            rrects[cls].height += rectList[i].height;
            foundWeights[cls] = max(foundWeights[cls], weights[i]);
            numInClass[cls]++;
        }

        for( i = 0; i < nclasses; i++ )
        {
            // find the average of all ROI in the cluster
            cv::Rect_<float> r = rrects[i];
            double s = 1.0/numInClass[i];
            rrects[i] = cv::Rect_<float>(cv::saturate_cast<float>(r.x*s),
                cv::saturate_cast<float>(r.y*s),
                cv::saturate_cast<float>(r.width*s),
                cv::saturate_cast<float>(r.height*s));
        }

        rectList.clear();
        weights.clear();

        for( i = 0; i < nclasses; i++ )
        {
            cv::Rect r1 = rrects[i];
            int n1 = numInClass[i];
            float w1 = foundWeights[i];
// 			if( n1 <= groupThreshold )
// 				continue;
            // filter out small rectangles inside large rectangles
// 			for( j = 0; j < nclasses; j++ )
// 			{
// 				int n2 = numInClass[j];
//
// 				if( j == i || n2 <= groupThreshold )
// 					continue;
//
// 				cv::Rect r2 = rrects[j];
//
// 				int dx = cv::saturate_cast<int>( r2.width * eps );
// 				int dy = cv::saturate_cast<int>( r2.height * eps );
//
// 				if( r1.x >= r2.x - dx &&
// 					r1.y >= r2.y - dy &&
// 					r1.x + r1.width <= r2.x + r2.width + dx &&
// 					r1.y + r1.height <= r2.y + r2.height + dy &&
// 					(n2 > std::max(3, n1) || n1 < 3) )
// 					break;
// 			}

            /*if( j == nclasses )*/
            {
                rectList.push_back(r1);
                weights.push_back(w1);
            }
        }
    }

    void HOG::detectMultiScale( Mat& img,vector<Rect>& foundLocations,vector<float>& foundWeights)
    {
        CV_Assert(img.data && (img.type() == CV_8UC1));

        const int nblocks = getNBlocks().area(); // количество блоков
        const int nblockDesc = getDescriptorSize()/nblocks; // длина вектора дл¤ каждого блока

        int levels = 0;
        int nlevels = 70;
        double scale = 1.;
        double scale0 = 0.97;


        vector<double> levelScale;
        for( levels = 0; levels < nlevels; levels++ )
        {
            levelScale.push_back(scale);
            if( cvRound(img.cols*scale) < _winSize.width ||
                cvRound(img.rows*scale) < _winSize.height ||
                scale0 >= 1 )
                break;
            scale *= scale0;
        }
        levels = std::max(levels, 1);
        levelScale.resize(levels);

        Mat imgStorage = Mat(img.rows,img.cols,img.type());
        cv::Size winStride = Size(2,2);
        Ptr<HOG::MemoryManager> memoryManager( new MemoryManager( this, imgStorage, winStride ) );

        foundLocations.clear(); // найденные пр¤моугольники
        foundWeights.clear(); // найденные веса

        vector<bool> tmpFind; // временный нашел, не нашел
        vector<float> tmpWeights; // временные веса
        vector<cv::Rect> tmpLocations; // временные пр¤моугольники

        for(int i = 0; i< levels; i++)
        {
            scale = levelScale[i];
            cv::Size sz = cv::Size(cvRound(img.cols*scale), cvRound(img.rows*scale));
            if(sz == img.size())
            {
                memoryManager->recount(img);
            }
            else
            {
                Mat scaleImg = Mat(sz,imgStorage.type(),(void*)(imgStorage.data));
                cv::resize(img,scaleImg,sz);
                memoryManager->recount(scaleImg);
            }
            precompWins(memoryManager);
            /*ищем бревна*/
            int nwin = memoryManager->wins.size().area(),
                win_height = memoryManager->wins.rows;
            if((int)tmpFind.size() < nwin)
            {
                tmpFind.resize(nwin);
                tmpWeights.resize(nwin);
                tmpLocations.resize(nwin);
            }


#ifdef _DEBUG
            RTreesInvoker rti(memoryManager->wins,
                nblocks,
                nblockDesc,
                _winSize,
                winStride,
                1.0/scale,
                _rtrees,
                &tmpFind,&tmpLocations,&tmpWeights);
            rti(Range(0,win_height));
#else
            parallel_for_(Range(0,win_height),RTreesInvoker(memoryManager->wins,
                nblocks,
                nblockDesc,
                _winSize,
                winStride,
                1.0/scale,
                _rtrees,
                &tmpFind,&tmpLocations,&tmpWeights));
#endif // _DEBUG
            /*копируем все найденые*/
            for(int j = 0; j < nwin; j++)
            {
                if(tmpFind[j] == true)
                {
                    foundLocations.push_back(tmpLocations[j]);
                    foundWeights.push_back(tmpWeights[j]);
                }
            }
        }
        _groupRectangles(foundLocations,foundWeights,1,0.15);
    }

    void HOG::loadRTrees(const string& filename, const string& name)
    {
        _rtrees->load(filename.c_str(),name.c_str());
    }
    /*------------------------------------------------------------------------*/
    /*манагер пам¤ти дл¤ hog*/
    /*------------------------------------------------------------------------*/
    HOG::MemoryManager::MemoryManager(HOG* hog_ptr,cv::Mat& img,cv::Size& win_stride)
    {
        /*инициализаци¤ полей*/
        this->_memPtr = (void*)0;
        this->_hogPtr = hog_ptr;
        this->_imgSize = img.size();
        this->image = img;
        if(win_stride == Size())
        {
            win_stride = _hogPtr->_cellSize;
        }
        win_stride = Size(cv::gcd(win_stride.width, _hogPtr->_blockStride.width),
                        cv::gcd(win_stride.height, _hogPtr->_blockStride.height));
        this->_winStride = win_stride;
        /*выделить пам¤ть*/
        this->recount(image);
    }
    HOG::MemoryManager::~MemoryManager()
    {
        _hogPtr = (HOG*)0;
        if(_memPtr)
        {
            cv::fastFree(_memPtr);
        }
    }

    void HOG::MemoryManager::recount(cv::Mat& newImg)
    {
        const int height = newImg.rows,
            width = newImg.cols,
            nbins = _hogPtr->_nbins;
        CV_Assert( (_imgSize.height >= height ) && (_imgSize.width >= width));
        this->image = newImg;
        cv::Size new_size = newImg.size();
        if(!_memPtr)
        {
            /*если еще не выделил нужно выделить пам¤ть*/
            _memPtr = cv::fastMalloc(	getXmapBytes(_imgSize) + getYmapBytes(_imgSize) + getIntegrBufBytes(_imgSize) +
                                        getGradsBytes(_imgSize) + getCellsBytes(_imgSize) +
                                        getBlocksBytes(_imgSize) + getWinsBytes(_imgSize)
                                        );
        }
        uchar* pt = (uchar*)_memPtr;
        /*xmap*/
        this->xmap = (int*)pt;
        pt += getXmapBytes(new_size);
        /*ymap*/
        this->ymap = (int*)pt;
        pt += getYmapBytes(new_size);
        /*буфер дл¤ интегрального изображени¤*/
        this->integrBuf = (float*)pt;
        pt += getIntegrBufBytes(new_size);
        /*градиенты*/
        this->grads = Mat((height + 1)*nbins,width + 1,CV_32FC1,(void*)pt);
        pt += getGradsBytes(new_size);
        /*¤чейки*/
        cv::Size ncells = getNCells(new_size);
        this->cells = Mat(ncells.height*nbins,ncells.width,CV_32FC1,(void*)pt);
        pt += getCellsBytes(new_size);
        /*блоки*/
        cv::Size nblocks = getNBlocks(new_size);
        int ncellsInBlock = _hogPtr->getNCells().area();
        this->blocks = Mat(nblocks.height,nblocks.width,CV_MAKETYPE(CV_32F,( ncellsInBlock * nbins )),(void*)pt);
        pt += getBlocksBytes(new_size);
        /*окна*/
        cv::Size nwins = getNWins(new_size);
        int nblocksInWin = _hogPtr->getNBlocks().area();
        this->wins = Mat(nwins.height,nwins.width,CV_MAKETYPE(CV_8U,nblocksInWin*sizeof(float*)),(void*)pt);
    }

    inline int HOG::MemoryManager::getGradsBytes(const cv::Size& imgSize) const
    {
        return (imgSize.height + 1)*(imgSize.width + 1)*(_hogPtr->_nbins)*sizeof(float);
    }

    inline int HOG::MemoryManager::getCellsBytes(const cv::Size& imgSize) const
    {
        cv::Size ncells = getNCells(imgSize);
        return ncells.height*ncells.width*(_hogPtr->_nbins)*sizeof(float);
    }

    inline int HOG::MemoryManager::getBlocksBytes(const cv::Size& imgSize) const
    {
        cv::Size nblocks = getNBlocks(imgSize);
        int ncellsInBlock = _hogPtr->getNCells().area();
        return nblocks.height * nblocks.width * ncellsInBlock * (_hogPtr->_nbins) * sizeof(float);
    }

    inline int HOG::MemoryManager::getWinsBytes(const cv::Size& imgSize) const
    {
        cv::Size nwins = getNWins(imgSize);
        /*количество блоков в окне*/
        int nblocksInWin =  _hogPtr->getNBlocks().area();
        return nwins.height * nwins.width * nblocksInWin * sizeof(float*);
    }

    inline int HOG::MemoryManager::getXmapBytes(const cv::Size& imgSize) const
    {
        return (imgSize.width + 2)*sizeof(int);
    }

    inline int HOG::MemoryManager::getYmapBytes(const cv::Size& imgSize) const
    {
        return (imgSize.height + 2)*sizeof(int);
    }

    inline int HOG::MemoryManager::getIntegrBufBytes(const cv::Size& imgSize) const
    {
        return (imgSize.width + 1) * _hogPtr->_nbins * 2 * sizeof(float);
    }

    inline cv::Size HOG::MemoryManager::getNCells(const cv::Size& imgSize) const
    {
        return Size((imgSize.width - _hogPtr->_cellSize.width)/_winStride.width + 1,
                    (imgSize.height - _hogPtr->_cellSize.height)/_winStride.height + 1);
    }

    inline cv::Size HOG::MemoryManager::getNBlocks(const cv::Size& imgSize) const
    {
        return Size((imgSize.width - _hogPtr->_blockSize.width)/_winStride.width + 1,
                    (imgSize.height - _hogPtr->_blockSize.height)/_winStride.height + 1);
    }

    inline cv::Size HOG::MemoryManager::getNWins(const cv::Size& imgSize) const
    {
        return Size((imgSize.width - _hogPtr->_winSize.width)/_winStride.width + 1,
            (imgSize.height - _hogPtr->_winSize.height)/_winStride.height + 1);
    }

    inline cv::Size HOG::MemoryManager::getWinStride() const
    {
        return _winStride;
    }


