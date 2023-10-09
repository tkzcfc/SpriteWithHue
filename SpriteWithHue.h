#pragma once

#include "axmol.h"

class SpriteWithHue: public ax::Sprite
{
public:
    static SpriteWithHue* create(const std::string& filename);
    static SpriteWithHue* create(const std::string& filename, const ax::Rect& rect);
    
    static SpriteWithHue* createWithTexture(ax::Texture2D *texture);
    static SpriteWithHue* createWithTexture(ax::Texture2D *texture, const ax::Rect& rect, bool rotated=false);
    
    static SpriteWithHue* createWithSpriteFrame(ax::SpriteFrame *spriteFrame);
    static SpriteWithHue* createWithSpriteFrameName(const std::string& spriteFrameName);
    
    float getHue();

    // [0,2¦Ð]
    void setHue(float hue);
    
protected:
    float _hue;
    GLfloat _mat[3][3];
    
    bool initWithTexture(ax::Texture2D *texture);
    bool initWithTexture(ax::Texture2D *texture, const ax::Rect& rect);
    virtual bool initWithTexture(ax::Texture2D *texture, const ax::Rect &rect, bool rotated);
    virtual bool initWithSpriteFrame(ax::SpriteFrame *spriteFrame);
    
    void setupDefaultSettings();
    void initShader();
    virtual void updateColor();
    void updateColorMatrix();
    void updateAlpha();
    GLfloat getAlpha();
    
    void hueUniformCallback(ax::backend::ProgramState* programState, ax::backend::UniformLocation uniform);
    void alphaUniformCallback(ax::backend::ProgramState* programState, ax::backend::UniformLocation uniform);
};

