#include "SpriteWithHue.h"

using namespace ax;

void xRotateMat(float mat[3][3], float rs, float rc);
void yRotateMat(float mat[3][3], float rs, float rc);
void zRotateMat(float mat[3][3], float rs, float rc);
void matrixMult(float a[3][3], float b[3][3], float c[3][3]);
void hueMatrix(GLfloat mat[3][3], float angle);
void premultiplyAlpha(GLfloat mat[3][3], float alpha);

SpriteWithHue* SpriteWithHue::create(const std::string& filename)
{
    SpriteWithHue *sprite = new (std::nothrow) SpriteWithHue();
    if (sprite && sprite->initWithFile(filename))
    {
        sprite->autorelease();
        return sprite;
    }
    AX_SAFE_DELETE(sprite);
    return nullptr;
}

SpriteWithHue* SpriteWithHue::create(const std::string& filename, const ax::Rect& rect)
{
    SpriteWithHue *sprite = new (std::nothrow) SpriteWithHue();
    if (sprite && sprite->initWithFile(filename, rect))
    {
        sprite->autorelease();
        return sprite;
    }
    AX_SAFE_DELETE(sprite);
    return nullptr;
}

SpriteWithHue* SpriteWithHue::createWithTexture(ax::Texture2D *texture)
{
    SpriteWithHue *sprite = new (std::nothrow) SpriteWithHue();
    if (sprite && sprite->initWithTexture(texture))
    {
        sprite->autorelease();
        return sprite;
    }
    AX_SAFE_DELETE(sprite);
    return nullptr;
}

SpriteWithHue* SpriteWithHue::createWithTexture(ax::Texture2D *texture, const ax::Rect& rect, bool rotated)
{
    SpriteWithHue *sprite = new (std::nothrow) SpriteWithHue();
    if (sprite && sprite->initWithTexture(texture, rect, rotated))
    {
        sprite->autorelease();
        return sprite;
    }
    AX_SAFE_DELETE(sprite);
    return nullptr;
}

SpriteWithHue* SpriteWithHue::createWithSpriteFrame(ax::SpriteFrame *spriteFrame)
{
    SpriteWithHue *sprite = new (std::nothrow) SpriteWithHue();
    if (sprite && spriteFrame && sprite->initWithSpriteFrame(spriteFrame))
    {
        sprite->autorelease();
        return sprite;
    }
    AX_SAFE_DELETE(sprite);
    return nullptr;
}

SpriteWithHue* SpriteWithHue::createWithSpriteFrameName(const std::string& spriteFrameName)
{
    ax::SpriteFrame *frame = ax::SpriteFrameCache::getInstance()->getSpriteFrameByName(spriteFrameName);
    
#if _AX_DEBUG > 0
    char msg[256] = { 0 };
    snprintf(msg, sizeof(msg), "Invalid spriteFrameName: %s", spriteFrameName.data());
    AXASSERT(frame != nullptr, msg);
#endif
    
    return createWithSpriteFrame(frame);
}

bool SpriteWithHue::initWithTexture(ax::Texture2D *texture, const ax::Rect &rect, bool rotated)
{
    setupDefaultSettings();
    initShader();
    bool ret = Sprite::initWithTexture(texture, rect, rotated);
    return ret;
}

bool SpriteWithHue::initWithTexture(ax::Texture2D *texture)
{
    AXASSERT(texture != nullptr, "Invalid texture for sprite");
    
    ax::Rect rect = ax::Rect::ZERO;
    rect.size = texture->getContentSize();
    
    return initWithTexture(texture, rect);
}

bool SpriteWithHue::initWithTexture(ax::Texture2D *texture, const ax::Rect& rect)
{
    return initWithTexture(texture, rect, false);
}

bool SpriteWithHue::initWithSpriteFrame(ax::SpriteFrame *spriteFrame)
{
    AXASSERT(spriteFrame != nullptr, "");
    
    bool bRet = initWithTexture(spriteFrame->getTexture(), spriteFrame->getRect());
    setSpriteFrame(spriteFrame);
    
    return bRet;
}

void SpriteWithHue::setupDefaultSettings()
{
    _hue = 0.0f;
}

void SpriteWithHue::initShader()
{
    std::string fragSource = R"(
    #ifdef GL_ES                                        
    precision mediump float;                            
    #endif

    #ifdef GL_ES
    varying mediump vec2 v_texCoord;
    #else
    varying vec2 v_texCoord;
    #endif

    uniform sampler2D u_tex0;
    uniform mat3 u_hue;                                 
    uniform float u_alpha;

    void main()                                         
    {                                                   
        vec4 pixColor = texture2D(u_tex0, v_texCoord); 
        vec3 rgbColor = u_hue * pixColor.rgb;               
        gl_FragColor = vec4(rgbColor, pixColor.a * u_alpha);
    }                                                   
    )";

#define USE_ONCE_PROGRAME 0

#if USE_ONCE_PROGRAME
    const int programType = 10001;

    ProgramManager::getInstance()->registerCustomProgramFactory(programType, positionTextureColor_vert, fragSource, [](Program* program) {
        auto vertexLayout = program->getVertexLayout();

        /// a_position
        vertexLayout->setAttribute(backend::ATTRIBUTE_NAME_POSITION,
            program->getAttributeLocation(backend::Attribute::POSITION),
            backend::VertexFormat::FLOAT3, 0, false);
        /// a_texCoord
        vertexLayout->setAttribute(backend::ATTRIBUTE_NAME_TEXCOORD,
            program->getAttributeLocation(backend::Attribute::TEXCOORD),
            backend::VertexFormat::FLOAT2, offsetof(V3F_C4B_T2F, texCoords), false);

        /// a_color
        vertexLayout->setAttribute(backend::ATTRIBUTE_NAME_COLOR, program->getAttributeLocation(backend::Attribute::COLOR),
            backend::VertexFormat::UBYTE4, offsetof(V3F_C4B_T2F, colors), true);
        vertexLayout->setStride(sizeof(V3F_C4B_T2F));
    });

    auto program = ProgramManager::getInstance()->getCustomProgram(programType);
#else
    auto program = ProgramManager::newProgram(positionTextureColor_vert, fragSource, [](Program* program) {
        auto vertexLayout = program->getVertexLayout();
        
        /// a_position
        vertexLayout->setAttribute(backend::ATTRIBUTE_NAME_POSITION,
            program->getAttributeLocation(backend::Attribute::POSITION),
            backend::VertexFormat::FLOAT3, 0, false);
        /// a_texCoord
        vertexLayout->setAttribute(backend::ATTRIBUTE_NAME_TEXCOORD,
            program->getAttributeLocation(backend::Attribute::TEXCOORD),
            backend::VertexFormat::FLOAT2, offsetof(V3F_C4B_T2F, texCoords), false);
        
        /// a_color
        vertexLayout->setAttribute(backend::ATTRIBUTE_NAME_COLOR, program->getAttributeLocation(backend::Attribute::COLOR),
            backend::VertexFormat::UBYTE4, offsetof(V3F_C4B_T2F, colors), true);
        vertexLayout->setStride(sizeof(V3F_C4B_T2F));
    });
#endif
    auto programState = new backend::ProgramState(program);
    setProgramState(programState);
    AX_SAFE_RELEASE(programState);
#if USE_ONCE_PROGRAME
#else
    AX_SAFE_RELEASE(program);
#endif

    updateColor();
}

void SpriteWithHue::updateColor()
{
    Sprite::updateColor();
    updateColorMatrix();
    updateAlpha();
}

void SpriteWithHue::hueUniformCallback(backend::ProgramState* programState, backend::UniformLocation uniform)
{
   programState->setUniform(uniform, _mat, sizeof(_mat));
}

void SpriteWithHue::alphaUniformCallback(backend::ProgramState* programState, backend::UniformLocation uniform)
{
    float alpha = getAlpha();
    programState->setUniform(uniform, &alpha, sizeof(alpha));
}

void SpriteWithHue::updateColorMatrix()
{
    hueMatrix(_mat, _hue);
    premultiplyAlpha(_mat, getAlpha());

    auto loc = _programState->getUniformLocation("u_hue");
    _programState->setCallbackUniform(loc, AX_CALLBACK_2(SpriteWithHue::hueUniformCallback, this));
}

void SpriteWithHue::updateAlpha()
{
    auto loc = _programState->getUniformLocation("u_alpha");
    _programState->setCallbackUniform(loc, AX_CALLBACK_2(SpriteWithHue::alphaUniformCallback, this));
}

GLfloat SpriteWithHue::getAlpha()
{
    return _displayedOpacity/255.0f;
}

float SpriteWithHue::getHue()
{
    return _hue;
}

void SpriteWithHue::setHue(float hue)
{
    _hue = hue;
    updateColorMatrix();
}

void xRotateMat(float mat[3][3], float rs, float rc)
{
    mat[0][0] = 1.0;
    mat[0][1] = 0.0;
    mat[0][2] = 0.0;
    
    mat[1][0] = 0.0;
    mat[1][1] = rc;
    mat[1][2] = rs;
    
    mat[2][0] = 0.0;
    mat[2][1] = -rs;
    mat[2][2] = rc;
}

void yRotateMat(float mat[3][3], float rs, float rc)
{
    mat[0][0] = rc;
    mat[0][1] = 0.0;
    mat[0][2] = -rs;
    
    mat[1][0] = 0.0;
    mat[1][1] = 1.0;
    mat[1][2] = 0.0;
    
    mat[2][0] = rs;
    mat[2][1] = 0.0;
    mat[2][2] = rc;
}


void zRotateMat(float mat[3][3], float rs, float rc)
{
    mat[0][0] = rc;
    mat[0][1] = rs;
    mat[0][2] = 0.0;
    
    mat[1][0] = -rs;
    mat[1][1] = rc;
    mat[1][2] = 0.0;
    
    mat[2][0] = 0.0;
    mat[2][1] = 0.0;
    mat[2][2] = 1.0;
}

void matrixMult(float a[3][3], float b[3][3], float c[3][3])
{
    int x, y;
    float temp[3][3];
    
    for(y=0; y<3; y++) {
        for(x=0; x<3; x++) {
            temp[y][x] = b[y][0] * a[0][x] + b[y][1] * a[1][x] + b[y][2] * a[2][x];
        }
    }
    for(y=0; y<3; y++) {
        for(x=0; x<3; x++) {
            c[y][x] = temp[y][x];
        }
    }
}

void hueMatrix(GLfloat mat[3][3], float angle)
{
#define SQRT_2      sqrt(2.0)
#define SQRT_3      sqrt(3.0)
    
    float mag, rot[3][3];
    float xrs, xrc;
    float yrs, yrc;
    float zrs, zrc;
    
    // Rotate the grey vector into positive Z
    mag = SQRT_2;
    xrs = 1.0/mag;
    xrc = 1.0/mag;
    xRotateMat(mat, xrs, xrc);
    mag = SQRT_3;
    yrs = -1.0/mag;
    yrc = SQRT_2/mag;
    yRotateMat(rot, yrs, yrc);
    matrixMult(rot, mat, mat);
    
    // Rotate the hue
    zrs = sin(angle);
    zrc = cos(angle);
    zRotateMat(rot, zrs, zrc);
    matrixMult(rot, mat, mat);
    
    // Rotate the grey vector back into place
    yRotateMat(rot, -yrs, yrc);
    matrixMult(rot,  mat, mat);
    xRotateMat(rot, -xrs, xrc);
    matrixMult(rot,  mat, mat);
}

void premultiplyAlpha(GLfloat mat[3][3], float alpha)
{
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            mat[i][j] *= alpha;
        }
    }
}
