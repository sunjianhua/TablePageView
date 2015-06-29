//
//  TablePageHeader.h
//
//  Created by 孙建华 on 2015.
//

#ifndef __Spine__TablePageHeader__
#define __Spine__TablePageHeader__

#include "cocos2d.h"

class TablePageView;

struct TablePageHeadInfo
{
    char _imageName[32];
    // 标签默认左上角为起点，设置这个值，如果是水平标签，那么标签的y默认点就为这个值，如果是垂直标签，那么标签的x默认点就为这个值
    float _imageBaseLine;
    char _title[32];
    
    char _infoBGName[32];
    char _info[32];
    float _infoX;
    float _infoY;
};

class TablePageHeader : public cocos2d::Node
{
public:
    TablePageHeader();
    virtual ~TablePageHeader();
    
    bool init(TablePageView* pTablePageView, int tablePageIndex);
    void setInfo(const char* pInfo);
    void setSelected(bool isSelected = false);
    
    //
    bool onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event *pEvent);
    void onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event *pEvent);
    
    //
    static TablePageHeader* create(TablePageView* pTablePageView, int tablePageIndex);
protected:
    const TablePageHeadInfo* _pPageHeadInfo;
    cocos2d::Sprite* _pImage;
    cocos2d::Sprite* _pInfoBG;
    TablePageView* _pTablePageView;
    int _tablePageIndex;
    bool _isSelected;
    cocos2d::Label* _pInfo;
};

#endif /* defined(__Spine__TablePageHeader__) */
