//
//  TablePageCell.h
//
//  Created by 孙建华 on 2015.
//
//

#ifndef __Spine__TablePageCell__
#define __Spine__TablePageCell__

#include "cocos2d.h"

/// 要做：支持子单元格，如果有子单元格，就按一组来处理
class TablePageCell : public cocos2d::Node
{
public:
    TablePageCell();
    virtual ~TablePageCell();
    
    bool init();
//    //
//    ssize_t getIdx() const;
//    void setIdx(ssize_t uIdx);
//    
//    void reset();
//    //
    static TablePageCell* create();
//
//protected:
//    ssize_t _idx;
};

#endif /* defined(__Spine__TablePageCell__) */
