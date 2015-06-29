//
//  TablePageHeader.cpp
//
//  Created by 孙建华 on 2015.
//

#include "TablePageHeader.h"
#include "TablePageView.h"

using namespace cocos2d;

TablePageHeader::TablePageHeader()
: _pPageHeadInfo(NULL)
, _pImage(NULL)
, _pInfoBG(NULL)
, _pTablePageView(NULL)
, _tablePageIndex(-1)
, _isSelected(false)
, _pInfo(NULL)
{
    CCLOG("TablePageHeader::TablePageHeader()");
}

TablePageHeader::~TablePageHeader()
{
    CCLOG("TablePageHeader::~TablePageHeader()");
}

bool TablePageHeader::init(TablePageView* pTablePageView, int tablePageIndex)
{
    if(!cocos2d::Node::init())
        return false;
    
    //
    const TablePage* pTablePage = pTablePageView->getTablePage(tablePageIndex);
    if(!pTablePage)
        return false;
    
    //    if(this != pTablePage->_pHead)
    //        return false;
    
    //
    _pTablePageView = pTablePageView;
    _tablePageIndex = tablePageIndex;
    
    Rect rect = Rect::ZERO;
    _pPageHeadInfo = &(pTablePage->_info._headInfo);
    if(strlen(_pPageHeadInfo->_imageName) > 1)
    {
        Texture2D *texture = Director::getInstance()->getTextureCache()->addImage(_pPageHeadInfo->_imageName);
        if (texture)
        {
            rect.size = texture->getContentSize();
            rect.size.width = rect.size.width / 2;
            _pImage =  cocos2d::Sprite::createWithTexture(texture, rect);
            _pImage->setAnchorPoint(Vec2(0, _pPageHeadInfo->_imageBaseLine / rect.size.height));
            addChild(_pImage);
        }
        else
        {
            CCASSERT(false, "图片加载异常");
        }
    }
    
    if(strlen(_pPageHeadInfo->_title) > 1)
    {
        Label* pTitle = Label::create();
        pTitle->setTag(123);
        pTitle->setSystemFontSize(20);
        pTitle->ignoreAnchorPointForPosition(true);
        pTitle->setDimensions(rect.size.width, rect.size.height);
        pTitle->setAlignment(TextHAlignment::CENTER, TextVAlignment::CENTER);
        pTitle->setString(_pPageHeadInfo->_title);
        addChild(pTitle);
    }
    
    if(strlen(_pPageHeadInfo->_info) > 1)
        setInfo(_pPageHeadInfo->_info);// 设置信息
    
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = CC_CALLBACK_2(TablePageHeader::onTouchBegan, this);
    listener->onTouchEnded = CC_CALLBACK_2(TablePageHeader::onTouchEnded, this);
    // listener->setSwallowTouches(true);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    
    //
    setContentSize(rect.size);
    
    return true;
}

void TablePageHeader::setInfo(const char* pInfo)
{
    if(!_pPageHeadInfo)
    {
        CCASSERT(false, "表格头异常");
        return;
    }
    
    if(!_pInfoBG)
    {
        _pInfoBG = Sprite::create(_pPageHeadInfo->_infoBGName);
        if(_pInfoBG)
        {
            _pInfoBG->setPosition(_pPageHeadInfo->_infoX, _pPageHeadInfo->_infoY);
            //_pInfoBG->setAnchorPoint(Vec2(0.5, 0.5));
            addChild(_pInfoBG);
        }
    }
    
    if(!_pInfo)
    {
        _pInfo = Label::create();
        _pInfo->setSystemFontSize(20);
        _pInfo->ignoreAnchorPointForPosition(true);
        if(_pInfoBG)
        {
            const Size& temp = _pInfoBG->getContentSize();
            _pInfo->setDimensions(temp.width, temp.height);
            _pInfo->setAlignment(TextHAlignment::CENTER, TextVAlignment::CENTER);
            _pInfoBG->addChild(_pInfo);
        }
        else
        {
            _pInfo->setPosition(_pPageHeadInfo->_infoX, _pPageHeadInfo->_infoY);
            addChild(_pInfo);
        }
    }
    
    if(strlen(pInfo) > 1)
    {
        if(_pInfo)
        {
            if(!_pInfoBG->isVisible())
                _pInfo->setVisible(true);
            
            _pInfo->setString(pInfo);
        }
        if(_pInfoBG && !_pInfoBG->isVisible())
            _pInfoBG->setVisible(true);
    }
    else
    {
        if(_pInfo)
            _pInfo->setVisible(false);
        if(_pInfoBG)
            _pInfoBG->setVisible(false);
    }
}

void TablePageHeader::setSelected(bool isSelected)
{
    if(!_pImage)
    {
        CCASSERT(false, "异常");
        return;
    }
    
    Rect rect = _pImage->getTextureRect();
    
    if(isSelected)
        rect.origin.x = rect.size.width;
    else
        rect.origin.x = 0;
    
    _pImage->setTextureRect(rect);
}

bool TablePageHeader::onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event *pEvent)
{
    Vec2 pos = convertToNodeSpace(pTouch->getLocation());
    
    const Size& size = getContentSize();
    Rect rect(0, 0, size.width, size.height);
    if(!rect.containsPoint(pos))
        return false;
    //
    if(_pTablePageView)
    {
        if(_tablePageIndex == _pTablePageView->getShowPageIndex())
            return false;
    }
    //
    return true;
}

void TablePageHeader::onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event *pEvent)
{
    if(!_pTablePageView)
        return;
    
    _pTablePageView->setHeaderSelected(_tablePageIndex);
    _pTablePageView->showPage(_tablePageIndex);
}

//
TablePageHeader* TablePageHeader::create(TablePageView* pTablePageView, int tablePageIndex)
{
    TablePageHeader *ret = new TablePageHeader();
    if (ret && ret->init(pTablePageView, tablePageIndex))
    {
        ret->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(ret);
    }
    
    return ret;
}
