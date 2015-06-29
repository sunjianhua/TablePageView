//
//  TablePageCell.cpp
//
//  Created by 孙建华 on 2015.
//
//

#include "TablePageCell.h"

using namespace cocos2d;

TablePageCell::TablePageCell()
// : _idx(CC_INVALID_INDEX)
{}

TablePageCell::~TablePageCell()
{}

bool TablePageCell::init()
{
    if(!Node::init())
        return false;
    
    return true;
}

//void TablePageCell::reset()
//{
//    _idx = CC_INVALID_INDEX;
//}
//
//ssize_t TablePageCell::getIdx() const
//{
//    return _idx;
//}
//
//void TablePageCell::setIdx(ssize_t idx)
//{
//    _idx = idx;
//}

//
TablePageCell* TablePageCell::create()
{
    TablePageCell *ret = new TablePageCell();
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
