//
//  ScrollBar.cpp
//
//  Created by 孙建华 on 2015.
//
//

#include "ScrollBar.h"

using namespace cocos2d;

ScrollBar::ScrollBar()
: _pBG(NULL)
, _pBar(NULL)
, _ratio(0.f)
, _barSize(20, 0)
{}

ScrollBar::~ScrollBar()
{}

bool ScrollBar::initWithSize(const cocos2d::Size& showSize, const cocos2d::Size& contentSize)
{
    if(!cocos2d::Node::init())
        return false;
    
    updateWithSize(showSize, contentSize);
    
    return true;
}

void ScrollBar::updateWithSize(const cocos2d::Size& showSize, const cocos2d::Size& contentSize)
{
    cocos2d::Size sSize = showSize;
    cocos2d::Size cSize = contentSize;
    if(cSize.width < sSize.width)
        cSize.width = sSize.width;
    if(cSize.height < sSize.height)
        cSize.height = sSize.height;
    
    setContentSize(Size(_barSize.width, sSize.height));
    
    if(!_pBG)
    {
        _pBG = LayerColor::create(Color4B(0, 0, 255, 160));
        addChild(_pBG);
    }
    _pBG->setContentSize(Size(_barSize.width, sSize.height));
    
    if(!_pBar)
    {
        _pBar = LayerColor::create(Color4B(0, 255, 0, 200));
        addChild(_pBar);
    }
    
    _ratio = sSize.height / cSize.height;
    
    float barHeight = _ratio * sSize.height;
    _pBar->setContentSize(Size(_barSize.width - 2, barHeight));
    _pBar->setPosition(1, sSize.height - barHeight);
}

void ScrollBar::setPos(cocos2d::Vec2& pos)
{
    if(!_pBar)
        return;
    
    float y = _ratio * pos.y;

    _pBar->setPositionY(-y);
    
    // runAction(Sequence::create(DelayTime::create(1), FadeOut::create(1)));
}

ScrollBar* ScrollBar::create(const cocos2d::Size& showSize, const cocos2d::Size& contentSize)
{
    ScrollBar *ret = new ScrollBar();
    if (ret && ret->initWithSize(showSize, contentSize))
    {
        ret->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(ret);
    }
    
    return ret;
}
