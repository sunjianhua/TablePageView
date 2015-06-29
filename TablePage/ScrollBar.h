//
//  ScrollBar.h
//
//  Created by 孙建华 on 2015.
//
//

#ifndef __Spine__ScrollBar__
#define __Spine__ScrollBar__

#include "cocos2d.h"

class ScrollBar : public cocos2d::Node
{
public:
    ScrollBar();
    virtual ~ScrollBar();
    
    bool initWithSize(const cocos2d::Size& showSize, const cocos2d::Size& contentSize);
    void updateWithSize(const cocos2d::Size& showSize, const cocos2d::Size& contentSize);

    void setPos(cocos2d::Vec2& pos);
    const cocos2d::Size& getBarSize(){return _barSize;}
    
    //
    static ScrollBar* create(const cocos2d::Size& showSize, const cocos2d::Size& contentSize);
    
protected:
    cocos2d::LayerColor* _pBG;
    cocos2d::LayerColor* _pBar;
    float _ratio;
    cocos2d::Size _barSize;
//    cocos2d::Size _showSize;
//    cocos2d::Size _contentSize;
};

#endif /* defined(__Spine__ScrollBar__) */
