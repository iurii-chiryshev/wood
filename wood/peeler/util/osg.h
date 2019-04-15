/**
 * @file
 * @brief Заголовочный файл с классами-адаптерами OSG библиотеки (только отрисовка сцены)
 * @author Chiryshev Iurii <iurii.chiryshev@mail.ru>
 * В проекте несколько библиотек работы с 3d. Одни нужны для отрисовки, другие, - для расчета.
 * Эти библиотеки огромные. Чтобы не запутатся во всем этом нужны точки входа в эти библиотеки,
 * притом, только тех вещей которые нужны
 * Адаптер над OSG lib
 * */
#ifndef UTIL_OSG_H
#define UTIL_OSG_H

#include "util/pcl.h"

#include <osg/Point>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Switch>
#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osg/Math>
#include <osg/Quat>
#include <osg/Material>
#include <osg/Matrixd>
#include <osg/MatrixTransform>
#include <osg/PolygonMode>
#include <osgGA/TrackballManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgQt/GraphicsWindowQt>
#include <osgViewer/Viewer>
#include <osgViewer/CompositeViewer>

namespace util {

/**
 * @brief The OSG struct
 * Типы и методы для работы с OSG - отрисовщиком сцены.
 * + т.к. алгоритмы заточены на Eigen и PLC, а рисовалка на OSG, нужна конвертация м/у pcl и osg
 *
 */
struct OSG
{
    typedef osg::Geode Geode;
    typedef osg::ref_ptr<Geode> GeodePtr;

    typedef osg::Node Node;
    typedef osg::ref_ptr<Node> NodePtr;

    typedef osg::Group Group;
    typedef osg::ref_ptr<Group> GroupPtr;

    typedef osg::Switch Switch;
    typedef osg::ref_ptr<Switch> SwitchPtr;

    typedef osg::Camera Camera;
    typedef osg::ref_ptr<Camera> CameraPtr;

    typedef osg::MatrixTransform Transform;
    typedef osg::ref_ptr<Transform> TransformPtr;

    typedef osgText::Text Text;
    typedef osg::ref_ptr<Text> TextPtr;

    OSG(){}
    /**
     * @brief createCamera
     * @param x
     * @param y
     * @param w
     * @param h
     * @return камера
     * Создать камеру
     */
    static Camera *createCamera( int x = 0, int y = 0, int w = 800, int h = 600 );
    /**
     * @brief createHUDCamera
     * @param left
     * @param right
     * @param bottom
     * @param top
     * @return hud камера т.е. не свободноболтающаяся
     * Создать hud камеру, например, для отображения текста,
     * чтобы при вращении сцены текст не вращался, а всегда смотрел на юзера
     */
    static Camera *createHUDCamera ( double left = 0, double right = 800, double bottom = 0, double top = 600 );

    /**
     * @brief blend
     * @param[in] src входной массив
     * @param[out] dst выходной массив
     * @param[in] bg цвет
     * @param[in] t коэффициент подмешивания
     * Подмешать какой-нить цвет в массив
     */
    static void blend(const osg::Vec4Array &src, osg::Vec4Array &dst, const osg::Vec4f &bg, float t = 0.5f);

    /**
     * @brief cloud2vertices
     * @param[in] cloud - облако точек
     * @param[out] vertices - массив вертексов
     * @param[out] colors - массив цветов
     * Конветртровать из PCL облакто точек в OSG облако для отрисовки
     */
    static void cloud2vertices(const PCL::PointCloud &cloud,osg::Vec3Array &vertices,osg::Vec4Array &colors);

    /**
     * @brief bboxNode
     * @param[in] cloud pcl облако точек
     * @param[out] size - размеры Width, Height, Depth
     * @return osg нода ограничевающего прямоугольника
     * Сделать визуальное представление для ограничивающегося прямоугольника
     */
    static OSG::NodePtr makeBBox(const PCL::PointCloud &cloud, osg::Vec3 &size);

    /**
     * @brief makeStateSet
     * @param point_size
     * @return
     */
    static osg::StateSet *makeStateSet(float point_size = 1.0f);

    /**
     * @brief makeGeometry
     * @param[in] cloud pcl облако точек
     * @param[in] bg - если нужно подкрасить
     * @param[in] t - если нужно подкасить выходное облако
     * @return - нода (облако точек), которую можно отобразить
     * Создать Geode ноду (Geometry - хранит данные для рисования) с облаком точек
     */
    static osg::Geometry *makeGeometry(const PCL::PointCloud &cloud,const osg::Vec4f &bg = osg::Vec4f(), float t = 0.5f);

    /**
     * @brief createText
     * @param[in] pos позиция текста
     * @param[in] content содержимое
     * @param[in] size размер текста
     * @param[in] color цвет текста
     * @return osg текст для отображения (не нода для отображения)
     * Создать текст для отображение в osg
     */
    static Text *makeText( const osg::Vec3 &pos = osg::Vec3(0,0,0),
                           const std::string &content = "",
                           float size = 10,
                           const osg::Vec4 &color = osg::Vec4(0.93f, 0.67f, 0.49f, 1.f) );

    /**
     * @brief makeTextGeoge
     * @param[in] pos позиция текста
     * @param[in] content сожержимое текста
     * @param[in] size размер текста
     * @param[in] color цвет текста
     * @return текстовая нода для отображения
     * Создать текстовую ноду для отображение в osg
     */
    static Geode *makeTextGeoge( const osg::Vec3 &pos = osg::Vec3(0,0,0),
                                 const std::string &content = "",
                                 float size = 10,
                                 const osg::Vec4 &color = osg::Vec4(0.93f, 0.67f, 0.49f, 1.f) );

};

}//namespace util



#endif // UTIL_OSG_H
