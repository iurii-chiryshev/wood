HOST_NAME = $$QMAKE_HOST.name
#-------------------------------------------------
#   Chiryshev Iurii <iurii.chiryshev@mail.ru>
#   DESKTOP-QRN46PP - home pc
#   DESKTOP-48BO0EE - work pc
#-------------------------------------------------
win32:contains(HOST_NAME,DESKTOP-QRN46PP|DESKTOP-48BO0EE) {
    message( "Chiryshev Iurii $$HOST_NAME detected" )
    #-------------------------------------------------
    #               OSG
    #-------------------------------------------------
    contains(HOST_NAME,DESKTOP-QRN46PP): OSG_DIR = 'C:\Program Files (x86)\osg-3.4.0\v140-x86' # home pc
    else: OSG_DIR = 'D:\lib\osg-3.4.0\v140-x86' # work pc
    OSG_LIB = $$OSG_DIR\lib
    OSG_INCLUDE = $$OSG_DIR\include
    #-------------------------------------------------
    #               PCL
    #-------------------------------------------------
    PCL_DIR = 'C:\Program Files (x86)\PCL 1.8.0'
    PCL_LIB = $$PCL_DIR\lib
    PCL_INCLUDE = $$PCL_DIR\include\pcl-1.8
    # 3rdParty Eigen
    PCL_EIGEN_INCLUDE = $$PCL_DIR\3rdParty\Eigen\eigen3
    # 3rdParty Boost
    PCL_BOOST_INCLUDE = $$PCL_DIR\3rdParty\Boost\include\boost-1_61
    PCL_BOOST_LIB = $$PCL_DIR\3rdParty\Boost\lib
    # 3rdParty openni
    PCL_OPENNI2_DIR = 'C:\Program Files (x86)\OpenNI2'
    PCL_OPENNI2_INCLUDE = $$PCL_OPENNI2_DIR\Include
    PCL_OPENNI2_LIB = $$PCL_OPENNI2_DIR\Lib
    # 3rdParty flann
    PCL_FLANN_INCLUDE = $$PCL_DIR\3rdParty\FLANN\include
    PCL_FLANN_LIB = $$PCL_DIR\3rdParty\FLANN\lib
    #-------------------------------------------------
    #               OpenCV
    #-------------------------------------------------
    OPENCV_DIR = 'D:\lib\opencv-2.4.13\build'
    OPENCV_LIB = $$OPENCV_DIR\x86\vc12\lib
    OPENCV_INCLUDE = $$OPENCV_DIR\include
    #-------------------------------------------------
    #               all together
    #-------------------------------------------------
    INCLUDEPATH += $$OSG_INCLUDE \
                   $$PCL_INCLUDE \
                   $$PCL_EIGEN_INCLUDE \
                   $$PCL_BOOST_INCLUDE \
                   $$PCL_OPENNI2_INCLUDE \
                   $$PCL_FLANN_INCLUDE \
                   $$OPENCV_INCLUDE

    QMAKE_LIBDIR += $$OSG_LIB \
                    $$PCL_LIB \
                    $$PCL_BOOST_LIB \
                    $$PCL_OPENNI2_LIB \
                    $$PCL_FLANN_LIB \
                    $$OPENCV_LIB

    CONFIG(debug, debug|release) {
        # debug и release библиотеки лежат в одном месте,
        # но отличаются суфиксами
        LIBS += $$files($$OSG_LIB\*340d.lib) \
                $$files($$PCL_LIB\*_debug.lib) \
                $$files($$PCL_BOOST_LIB\*-mt-gd-1_61.lib) \
                $$files($$OPENCV_LIB\*2413d.lib)
    } else {
        LIBS += $$files($$OSG_LIB\*340.lib) \
                $$files($$PCL_LIB\*_release.lib) \
                $$files($$PCL_BOOST_LIB\*-mt-1_61.lib) \
                $$files($$OPENCV_LIB\*2413.lib)
    }

    #DEFINES += NO_BACKGROUND
} # Chiryshev Iurii PCs
