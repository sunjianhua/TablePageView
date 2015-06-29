//
//  TablePageDelegate.h
//
//  Created by 孙建华 on 2015.
//
//

#ifndef __Spine__TablePageDelegate__
#define __Spine__TablePageDelegate__

#include "cocos2d.h"

class TablePage;
class TablePageCell;

class TablePageDelegate
{
public:
    TablePageDelegate();
    virtual ~TablePageDelegate();
    
    //
    // 这个还是需要的，比如点击单元格，单元格变大，显示额外信息
    // virtual cocos2d::Size tablePageCellSize(TablePage* pTablePage, int row, int col) = 0;
    // virtual cocos2d::Size tablePageSize(TablePage* pTablePage);
    virtual TablePageCell* tablePageCellAtRowCol(TablePage* pTablePage, int row, int col) = 0;
    virtual ssize_t numberOfCellsInPage(TablePage* pTablePage) = 0;
    // 这个是否可以优化掉？？？
    virtual ssize_t numberOfPage(TablePage* pTablePage) = 0;
    virtual void cellTouched(TablePage* pTablePage, TablePageCell* pTablePageCell) = 0;
};

#endif /* defined(__Spine__TablePageDelegate__) */
