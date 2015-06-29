//
//  Restaurant.cpp
//  Spine
//
//  Created by 孙建华 on 15-1-21.
//
//

#include "Restaurant.h"
#include "TablePageView.h"
#include "TablePageCell.h"

using namespace cocos2d;

Restaurant::Restaurant()
{
    CCLOG("Restaurant::Restaurant()");
}

Restaurant::~Restaurant()
{
    CCLOG("Restaurant::Restaurant()");
}

bool Restaurant::init()
{
    if(!Layer::init())
        return false;
    
    TablePageView* pTablePageView = TablePageView::create(this, Size(400, 400));
    addChild(pTablePageView, pTablePageView->getLocalZOrder(), 12345678);
    
    pTablePageView->setPosition(Vec2(100, 100));
    
//    char _imageName[32];
//    // 标签默认左上角为起点，设置这个值，如果是水平标签，那么标签的y默认点就为这个值，如果是垂直标签，那么标签的x默认点就为这个值
//    float _imageBaseLine;
//    char _title[32];
//    
//    char _infoBGName[32];
//    char _info[32];
//    float _infoX;
//    float _infoY;
    TablePageInfo page = {0, 0, 50, 2, 0, 0, 158, 78, 10, 10, "当前显示‘测试’标签",
        {"ui//header.png", 0, "测试", "ui//infoBG.png", "11", 116, 60}};
    pTablePageView->addPage(page);
    TablePageInfo page1 = {1, 0, 50, 2, 0, 0, 158, 78, 10, 10, "当前显示‘测试测试’标签",
        {"ui//header.png", 0, "测试测试", "ui//infoBG.png", "11", 116, 60}};
    pTablePageView->addPage(page1);
    
    return true;
}

//cocos2d::Size Restaurant::tablePageCellSize(TablePage* pTablePage, int row, int col)
//{
//    return Size(20, 20);
//}

TablePageCell* Restaurant::tablePageCellAtRowCol(TablePage* pTablePage, int row, int col)
{
    auto string = String::createWithFormat("%ld", row);
    
    TablePageView* pTablePageView = dynamic_cast<TablePageView*>(getChildByTag(12345678));
    TablePageCell* pCell = pTablePageView->dequeueCell();
    if(!pCell)
    {
        pCell = TablePageCell::create();
        Sprite* pSprite = Sprite::create("testBG.png");
        pSprite->setAnchorPoint(Vec2(0, 0));
        pSprite->ignoreAnchorPointForPosition(true);
        pCell->addChild(pSprite);
        
        auto label = Label::createWithSystemFont(string->getCString(), "Helvetica", 20.0);
        label->setPosition(Vec2::ZERO);
        label->setAnchorPoint(Vec2::ZERO);
        label->setTag(123);
        pCell->addChild(label);
    }else
    {
        auto label = dynamic_cast<Label*>(pCell->getChildByTag(123));
        label->setString(string->getCString());
    }
    
    return pCell;
}

ssize_t Restaurant::numberOfCellsInPage(TablePage* pTablePage)
{
    return 20;
}

ssize_t Restaurant::numberOfPage(TablePage* pTablePage)
{
    return 3;
}

void Restaurant::cellTouched(TablePage* pTablePage, TablePageCell* pTablePageCell)
{
    CCLOG("当被点击处理");
    
//    TablePageView* pTablePageView = dynamic_cast<TablePageView*>(getChildByTag(12345678));
//    const std::vector<TablePageCell*>& cells = pTablePageView->getShowCell();
//    int i = 0;
//    for(; i < cells.size(); i++)
//    {
//        cells[i]->setPositionX(cells[i]->getPositionX() + 60);
//    }
}

//
Restaurant* Restaurant::create()
{
    Restaurant *ret = new Restaurant();
    if (ret && ret->init())
    {
        ret->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(ret);
    }
    
    return ret;
}
