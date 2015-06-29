//
//  Restaurant.h
//
//  Created by 孙建华 on 15.
//
//

#ifndef __Spine__Restaurant__
#define __Spine__Restaurant__

#include "cocos2d.h"
#include "TablePageDelegate.h"

class Restaurant : public cocos2d::Layer, public TablePageDelegate
{
public:
    Restaurant();
    virtual ~Restaurant();
    
    virtual bool init();
    
    // 测试用
    // virtual cocos2d::Size tablePageCellSize(TablePage* pTablePage, int row, int col);
    virtual TablePageCell* tablePageCellAtRowCol(TablePage* pTablePage, int row, int col);
    virtual ssize_t numberOfCellsInPage(TablePage* pTablePage);
    virtual ssize_t numberOfPage(TablePage* pTablePage);
    virtual void cellTouched(TablePage* pTablePage, TablePageCell* pTablePageCell);
    
    //
    static Restaurant* create();
    
protected:
};

#endif /* defined(__Spine__Restaurant__) */
