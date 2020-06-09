#include "qmlopenglitem.h"
#include <QPainter>
#include <QQuickWindow>
#include <QFile>
QmlOpenglItem::QmlOpenglItem(QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , m_render(new GlVideoRenderer(width(), height()))
{
    initAvGl();
    //绘制一帧yuv
    QFile file(":/img_320x240.yuv");
    if(file.open(QFile::ReadOnly)){
        QByteArray yuv = file.readAll();
        deliverFrame(false, false, yuv, 320, 240);
    }

}

void QmlOpenglItem::initAvGl()
{
    setRenderTarget(FramebufferObject);
    connect(this, SIGNAL(frameDelivered()), this, SLOT(renderFrame()));
    connect(this, SIGNAL(windowChanged(QQuickWindow*)), this, SLOT(handleWindowChanged(QQuickWindow*)));

    m_frame = new YuvFrame;
    m_frame->data = QByteArray();

}

QmlOpenglItem::~QmlOpenglItem()
{
    qDebug()<<__FUNCTION__;
    cleanup();
}

void QmlOpenglItem::setKeepAspectRatio(bool f){
    if(f != m_bkeepAspectRatio){
        qDebug()<<__FUNCTION__<<m_bkeepAspectRatio<<f;
        m_bkeepAspectRatio = f;
        emit sigKeepAspectRatioChanged(f);
    }
}


int QmlOpenglItem::setViewProperties(int zOrder, float left, float top, float right, float bottom)
{
    std::lock_guard<std::mutex> lock(m_stdmutex);
    if (m_render) {
        return m_render->setStreamProperties(zOrder, left, top, right, bottom);
    }
    return -1;
}

void QmlOpenglItem::handleWindowChanged(QQuickWindow *win)
{
    if (win){
        connect(win, SIGNAL(sceneGraphInvalidated()), this, SLOT(cleanup()), Qt::DirectConnection);
    }
}

void QmlOpenglItem::cleanup()
{
    {
        std::lock_guard<std::mutex> lock(m_stdmutex);
        if (m_frame){
            m_frame->data = QByteArray();
        }
    }
    m_render.reset();
    emit widgetInvalidated();
}

void QmlOpenglItem::setGeometry(const QRect &r)
{
    setX(r.x());
    setY(r.y());
    setWidth(r.width());
    setHeight(r.height());
}

void QmlOpenglItem::resize(const QSize &s)
{
    setWidth(s.width());
    setHeight(s.height());
}

void QmlOpenglItem::move(int x, int y)
{
    setX(x);
    setY(y);
}

void QmlOpenglItem::renderFrame()
{
    //    qDebug()<<__FUNCTION__<<"alvin!";
    update();
}

int QmlOpenglItem::deliverFrame(int rotation, bool mirrored, const QByteArray yuv, int width, int height)
{
    if (yuv.isNull() || width<=0 || height<=0)
        return -1;
    {
        std::lock_guard<std::mutex> lock(m_stdmutex);
        m_rotation = rotation;
        m_mirrored = mirrored;
        m_frame->width = width;
        m_frame->height = height;
        m_frame->stride = width;
        m_frame->data = yuv;
        //        m_frame->data.detach();
        if(width != m_render->width() || height != m_render->height()){
            m_render->cleanup();//大小变化之后，就需要重新初始化
        }
    }

    //notify the render thread to redraw the incoming frame
    emit frameDelivered();

    return 0;
}

void QmlOpenglItem::paint(QPainter *painter)
{
    if (!m_render || !isVisible() || m_frame->IsZeroSize())
        return;
    if(isKeepAspectRatio()){
        //不拉伸
        m_render->setRenderMode(GlVideoRenderer::KeepAspectRatio);
    }else{
        //拉伸
        m_render->setRenderMode(GlVideoRenderer::IgnoreAspectRatio);
    }

    std::lock_guard<std::mutex> lock(m_stdmutex);
    painter->beginNativePainting();

    if (!m_render->isInitialized())
        m_render->initialize(width(), height());
    m_render->setFrameInfo(m_rotation, m_mirrored);
    if (m_frame)
        m_render->renderFrame(*m_frame);

    painter->endNativePainting();
}

void QmlOpenglItem::geometryChanged(const QRectF& newGeometry, const QRectF& oldGeometry)
{
    if (newGeometry.width() != oldGeometry.width() ||
            newGeometry.height() != oldGeometry.height())
    {

        if(m_render)
            m_render->setSize(newGeometry.width(), newGeometry.height());
        emit viewSizeChanged(newGeometry.width(), newGeometry.height());
    }
    QQuickPaintedItem::geometryChanged(newGeometry, oldGeometry);
}



