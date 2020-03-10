#ifndef QMLOPENGLITEM_H
#define QMLOPENGLITEM_H

#include <QtCore/qglobal.h>
#include "glvideorender.h"
#include <mutex>
#include <memory>
#include <QQuickPaintedItem>
/**
 * @brief The QmlOpenglItem class
 * 使用opengl 绘制yuv数据的qml控件
 * @autor Alvin
 */
class QmlOpenglItem: public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(bool keepAspectRatio READ isKeepAspectRatio WRITE setKeepAspectRatio NOTIFY sigKeepAspectRatioChanged)
public:
    QmlOpenglItem(QQuickItem *parent = nullptr);
    ~QmlOpenglItem();
    /**
     * @brief deliverFrame//绘制一帧YUV
     * @param rotation 是否旋转
     * @param mirrored 是否翻转
     * @param yuv   yuv数据
     * @param width 图像宽
     * @param height 图像高
     * @return 0成功 -1失败
     */
    int deliverFrame(int rotation, bool mirrored, const QByteArray yuv, int width, int height);
protected:
    virtual void paint(QPainter *painter) Q_DECL_OVERRIDE;
    int setViewProperties(int zOrder, float left, float top, float right, float bottom);
    //是否拉伸显示，true：不拉伸，false：拉伸
    void setKeepAspectRatio(bool f);
    bool isKeepAspectRatio() const{return m_bkeepAspectRatio;}
protected:
    virtual void geometryChanged(const QRectF& newGeometry, const QRectF& oldGeometry) Q_DECL_OVERRIDE;
signals:
    void sigKeepAspectRatioChanged(bool f);
    void frameDelivered();
    void widgetInvalidated();
    void viewSizeChanged(int width, int height);
    void localVideoChanged();
public slots:
    void renderFrame();
    void handleWindowChanged(QQuickWindow *win);
    void cleanup();
private:
    bool m_bkeepAspectRatio = false;
    std::unique_ptr<GlVideoRenderer> m_render;
    std::mutex m_stdmutex;
    //usage of m_frame should be guarded by m_mutex
    YuvFrame* m_frame;
    int m_rotation;
    bool m_mirrored;
private:
    void initAvGl();
public:
    void setGeometry(const QRect & r);
    void resize(const QSize &s );
    void move(int x, int y);
};


#endif // QMLOPENGLITEM_H
