#ifndef GLVIDEORENDER_H
#define GLVIDEORENDER_H

#include <QOpenGLFunctions>
#include <mutex>

struct YuvFrame{
    int width;
    int height;
    QByteArray data;
    int stride;
    bool IsZeroSize() const {
         return (width==0 || height==0|| data.isEmpty());
    }
    unsigned char* y() const{
        unsigned char* yData = (unsigned char*)data.data();
        return yData;
    }
    unsigned char* u()const{
        const long len = width*height;
        unsigned char* yData = (unsigned char*)data.data();
        return yData + len;
    }
    unsigned char* v()const{
      const long len = width*height;
      unsigned char* yData = (unsigned char*)data.data();
      return yData + len*5/4;
    }
};

class AVideoWidget;
class QOpenGLShaderProgram;

class GlVideoRenderer
{
public:
    GlVideoRenderer(int width, int height);
    ~GlVideoRenderer();
    enum RenderMode{
        KeepAspectRatioByExpanding=1,// the image is scaled to a rectangle as small as possible outside size, preserving the aspect ratio.整张绘制，不压缩，不拉伸。可能不会全部展示
        KeepAspectRatio=2, // the image is scaled to a rectangle as large as possible inside size, preserving the aspect ratio。压缩以适应窗口全部展示。
        IgnoreAspectRatio=3 //the image is scaled to size. 自适应
    };

    bool isInitialized() const { return m_program != nullptr; }
    int initialize(int width, int height);
    void setSize(int width, int height);
    int setStreamProperties(int zOrder, float left, float top, float right, float bottom);
    int getStreamProperties(int& zOrder, float& left, float& top, float& right, float& bottom);
    int renderFrame(const YuvFrame& videoFrame);
    int width() const { return m_targetWidth; }
    int height() const { return m_targetHeight; }
    void setFrameInfo(int rotation, bool mirrored);
    void setRenderMode(RenderMode m){
        m_renderMode = m;
    }
    RenderMode renderMode() const{return m_renderMode;}
private:
    int prepare();
    int frameSizeChange(int width, int height);
    QOpenGLShaderProgram* createProgram();
    int applyVertices();
    void setupTextures(const YuvFrame& frameToRender);
    void initializeTexture(int name, int id, int width, int height);
    void updateTextures(const YuvFrame& frameToRender);
    void glTexSubImage2D(GLsizei width, GLsizei height, int stride, const uint8_t* plane);
    int ajustVertices();
    int adjustCoordinates(int frWidth, int frHeight, int targetWidth, int targetHeight, int renderMode);
    static QOpenGLFunctions* renderer();
    void cleanup();
private:
    std::mutex m_mutex;
    QOpenGLShaderProgram *m_program;
    GLfloat m_vertices[20];
    GLuint m_textureIds[3]; // Texture id of Y,U and V texture.

    int m_zOrder;
    float m_left;
    float m_top;
    float m_right;
    float m_bottom;
    RenderMode m_renderMode;

    int m_textureWidth;
    int m_textureHeight;
    int m_targetWidth;
    int m_targetHeight;
    bool m_resetGlVert;
    int m_rotation;
    bool m_mirrored;
};
#endif // GLVIDEORENDER_H
