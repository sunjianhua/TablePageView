//
//  TablePageView.cpp
//
//  Created by 孙建华 on 2015.
//
//

#include "TablePageView.h"
#include "TablePageDelegate.h"
#include "TablePageCell.h"
#include "ScrollBar.h"

using namespace cocos2d;

const float INVALID_SCROLL_POS = -1;

TablePageView::TablePageView(TablePageDelegate* pDelegate)
: _pDelegate(pDelegate)
, _moveDirection(Direction::NONE)
, _pageHeadHeight(0)
, _pClippingNode(NULL)
, _isScrollEvent(true)
, _pSelCellInfo(NULL)
, _pInfoLayer(NULL)
, _pLP(NULL)
, _pRP(NULL)
, _pScrollBar(NULL)
{
    CCASSERT(_pDelegate, "TablePageView异常，缺少TablePageDelegate");
    
    memset(&_showPageInfo, -1, sizeof(_showPageInfo));
}

TablePageView::~TablePageView()
{
    CCLOG("没有清理分配空间");
}

bool TablePageView::initWithSize(const cocos2d::Size& size, int pageHeadHeight)
{
    if(!Node::init())
        return false;
    
    _pageHeadHeight = pageHeadHeight;
    
    setContentSize(size);
    
    //
    cocos2d::Rect clippingRectangle(0, 0, size.width, size.height - pageHeadHeight);
    
    _pClippingNode = cocos2d::ClippingRectangleNode::create(clippingRectangle);
    _pClippingNode->setAnchorPoint(Vec2::ZERO);
    _pClippingNode->setPosition(0, pageHeadHeight);
    _pClippingNode->setContentSize(clippingRectangle.size);
    addChild(_pClippingNode);
    //
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = CC_CALLBACK_2(TablePageView::onTouchBegan, this);
    listener->onTouchMoved = CC_CALLBACK_2(TablePageView::onTouchMoved, this);
    listener->onTouchEnded = CC_CALLBACK_2(TablePageView::onTouchEnded, this);
    listener->setSwallowTouches(true);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    //
    return true;
}

void TablePageView::addPage(const TablePageInfo& info)
{
    TablePage* pPage = new TablePage;
    pPage->_pHead = NULL;
    pPage->_curPage = 0;
    int totalPageNum = _pDelegate->numberOfPage(pPage);
    pPage->_scrollPos.resize(totalPageNum);
    for(int i = 0; i < totalPageNum; i++)
        pPage->_scrollPos[i] = Vec2(INVALID_SCROLL_POS, INVALID_SCROLL_POS);// 没处理totalPageNum在其它函数改变后的对应变化，比如删除内容，页数减少
    memcpy(&pPage->_info, &info, sizeof(TablePageInfo));
    
    SameDataTypeTablePage* pSameData = findSameDataType(info._dataType);
    if(!pSameData)
    {
        pSameData = new SameDataTypeTablePage;
        pSameData->_dataType = info._dataType;
        pSameData->_pLayer = LayerColor::create(Color4B(255, 0, 0, 255));
        Sprite* pTest = Sprite::create("test.png");
        pTest->setTag(7654321);
        pTest->ignoreAnchorPointForPosition(true);
        pSameData->_pLayer->addChild(pTest);
        pSameData->_pLayer->setVisible(false);
        if(_pClippingNode)
            _pClippingNode->addChild(pSameData->_pLayer);
        
        _tablePages.push_back(pSameData);
    }
    pSameData->_pages.push_back(pPage);
    _tablePagesPos.push_back(std::pair<SameDataTypeTablePage*, TablePage*>(pSameData, pPage));
    
    // 因为_showPageIndex，取的是tablePage入堆的顺序，所以只能放到这处理了，要把_showPageIndex取值相关的重构，这个也要相应做处理
    pPage->_pHead = TablePageHeader::create(this, _tablePagesPos.size() - 1);
    if(pPage->_pHead)
    {
        addChild(pPage->_pHead);
        pPage->_pHead->setPosition(0, _pClippingNode->getContentSize().height);
        AdjustHeader();
    }
    
    //
    if(_tablePagesPos.size() < 2)
    {
        if(pPage->_pHead)
            pPage->_pHead->setSelected(true);
        
        showPage(0);
    }
}

/// 要做：_cellPos 这个可以优化，可以每页保存一个，不用每次都计算
/// 要做：支持单元格分组，分组的单元格有组头，点击组头可以折叠
void TablePageView::showPage(int pageIndex)
{
    //    if(_showPageInfo._showPageIndex == pageIndex)
    //        return;
    
    if(_tablePagesPos.empty())
        return;
    
    ssize_t len = _tablePagesPos.size();
    if((pageIndex < 0) || (pageIndex >= len))
    {
        CCASSERT(false, "页数索引异常！！！");
        return;
    }
    
    TablePage* pPage = _tablePagesPos[pageIndex].second;
    
    int totalPageNum = _pDelegate->numberOfPage(pPage);
    if((pPage->_curPage < 0) || (pPage->_curPage >= totalPageNum))
    {
        CCASSERT(false, "页数异常！！！");
        return;
    }
    
    const ssize_t cellNum = _pDelegate->numberOfCellsInPage(pPage);
    if((cellNum <= 0) || (cellNum > pPage->_info._rowNum * pPage->_info._colNum))
    {
        CCASSERT(false, "页单元格数异常！！！");
        return;
    }
    
    //
    //_showPageIndex = pageIndex;
    //
    // 需要把_cellPos._pCell 从SameDataTypeTablePage的_cellsUsed移到_cellsFreed
    _cellPos.resize(cellNum);
    //
    int rowNum = cellNum / pPage->_info._colNum;
    rowNum = rowNum <= 0 ? 1 : rowNum;
    int colNum = cellNum < pPage->_info._colNum ? cellNum : pPage->_info._colNum;
    
    //
    if(_cellAniInfo._hasAni)
        _cellAniInfo._pos.resize(rowNum);
    //
    Size totalSize(pPage->_info._leftMargin + colNum * pPage->_info._cellWidth + colNum * pPage->_info._cellHSpacing,
                   pPage->_info._topMargin + rowNum * pPage->_info._cellHeight + rowNum * pPage->_info._cellVSpacing);
    const Size& clippingRect = _pClippingNode->getContentSize();
    float clippingWidth = clippingRect.width;
    
    if(totalSize.width < clippingWidth)
        totalSize.width = clippingWidth;
    if(totalSize.height < clippingRect.height)
        totalSize.height = clippingRect.height;
    //
    //totalSize.setSize(512, 512);
    Vec2& scrollPos = pPage->_scrollPos[pPage->_curPage];
    // 设置初始值
    if(scrollPos.x < 0 && scrollPos.y < 0)
    {
        scrollPos.x = 0;
        scrollPos.y = -(totalSize.height - clippingRect.height);
    }
    //
    Node* pLayer = _tablePagesPos[pageIndex].first->_pLayer;
    pLayer->setContentSize(totalSize);
    pLayer->setVisible(true);
    pLayer->setPositionY(scrollPos.y);
    //
    CellAniInfo::PosInfo* pCellAniPos = NULL;
    CellPosInfo* pCellPosInfo = NULL;
    int index = 0;
    float rowPos = totalSize.height - pPage->_info._topMargin;
    float colPos = 0;
    for(int i = 0; i < rowNum; i++)
    {
        if(_cellAniInfo._hasAni)
            _cellAniInfo._pos[i].resize(colNum);
        //
        //rowPos = totalSize.height - (pPage->_info._topMargin + i * pPage->_info._cellHeight + i * pPage->_info._cellVSpacing);
        colPos = pPage->_info._leftMargin;
        for(int j = 0; j < colNum; j++)
        {
            pCellPosInfo = &_cellPos[index];
            pCellPosInfo->_isShow = false;
            pCellPosInfo->_row = i;
            pCellPosInfo->_col = j;
            pCellPosInfo->_pCell = NULL;
            
            pCellPosInfo->_left = colPos;//pPage->_info._leftMargin + j * pPage->_info._cellWidth + j * pPage->_info._cellHSpacing;
            pCellPosInfo->_right = pCellPosInfo->_left + pPage->_info._cellWidth;
            
            pCellPosInfo->_top = rowPos;
            pCellPosInfo->_bottom = rowPos - pPage->_info._cellHeight;
            //
            index++;
            colPos += pPage->_info._cellWidth + pPage->_info._cellHSpacing;
            //
            if(_cellAniInfo._hasAni)
            {
                pCellAniPos = & _cellAniInfo._pos[i][j];
                pCellAniPos->_pCellPosInfo = pCellPosInfo;
                
                if(_cellAniInfo._direction == PMD_Left)
                {
                    pCellAniPos->_x = pCellPosInfo->_left + clippingWidth;
                }
                else if(_cellAniInfo._direction == PMD_Right)
                {
                    pCellAniPos->_x = pCellPosInfo->_left - clippingWidth;
                }
                else
                {
                    CCASSERT(false, "移动方向异常");
                }
            }
        }
        rowPos -= (pPage->_info._cellHeight + pPage->_info._cellVSpacing);
    }
    
    //
    updateShowPageInfo(pageIndex);
    //
    didScroll();
    //
    updateScrollBar();
    //
    updateInfoLayer();
    //
    updateLRP();
}

//void TablePageView::updateCellPos()
//{}

/// 潜在错误：目前的处理在做动画时翻页：1、等待的动画可以看到。2、造成_cellsFreed冗余太多，可以监测这个值
void TablePageView::didScroll()
{
    TablePage* pTPage = _tablePagesPos[_showPageInfo._showPageIndex].second;
    
    SameDataTypeTablePage* pSameData = _tablePagesPos[_showPageInfo._showPageIndex].first;
    //
    float bottom = fabs(pTPage->_scrollPos[pTPage->_curPage].y);
    float top = bottom + _pClippingNode->getContentSize().height;
    float left = 0;// pSameData->_pLayer->getPositionX();
    float right = left + _pClippingNode->getContentSize().width;
    //
    CellPosInfo* pCellPos = NULL;
    
    ssize_t len = _cellPos.size();
    for(int i = 0; i < len; i++)
    {
        pCellPos = &_cellPos[i];
        if((pCellPos->_left > right) || (pCellPos->_right < left) || (pCellPos->_top < bottom) || (pCellPos->_bottom > top))
        {
            if(pCellPos->_isShow)
            {
                pSameData->_cellsUsed.erase(std::find(pSameData->_cellsUsed.begin(), pSameData->_cellsUsed.end(), pCellPos->_pCell));
                pSameData->_cellsFreed.push_back(pCellPos->_pCell);
                pCellPos->_pCell->retain();// 对应dequeueCell里的autorelease？？？
                pCellPos->_pCell->removeFromParentAndCleanup(true);
                unsigned int refNum = pCellPos->_pCell->getReferenceCount();
                if(refNum > 1)
                {
                    CCASSERT(false, "fuck");
                }
                pCellPos->_pCell = NULL;
                
                pCellPos->_isShow = false;
            }
        }
        else
        {
            if(!pCellPos->_isShow)
            {
                pCellPos->_pCell = _pDelegate->tablePageCellAtRowCol(pTPage, pCellPos->_row, pCellPos->_col);
                if(pCellPos->_pCell)
                {
                    if(_cellAniInfo._hasAni)
                    {
                        pCellPos->_pCell->setPosition(_cellAniInfo._pos[pCellPos->_row][pCellPos->_col]._x,
                                                      pCellPos->_bottom);
                    }
                    else
                        pCellPos->_pCell->setPosition(pCellPos->_left, pCellPos->_bottom);
                    pSameData->_pLayer->addChild(pCellPos->_pCell);
                    pSameData->_cellsUsed.push_back(pCellPos->_pCell);
                    
                    pCellPos->_isShow = true;
                }
            }
        }
    }
}

TablePageCell* TablePageView::dequeueCell()
{
    if(_tablePagesPos.empty())
        return NULL;
    
    std::vector<TablePageCell*>& freed = _tablePagesPos[_showPageInfo._showPageIndex].first->_cellsFreed;
    if(freed.empty())
        return NULL;
    
    TablePageCell* pCell = freed[0];
    // pCell->retain();
    freed.erase(freed.begin());
    pCell->autorelease();
    
    return pCell;
}

const std::vector<TablePageCell*>& TablePageView::getShowCell()
{
    return _tablePagesPos[_showPageInfo._showPageIndex].first->_cellsUsed;
}

/// 潜在错误：应该有一个处理把pTouch->getLocation()，转换为本地节点坐标再做处理，这个因为父节点为全屏显示所以会判断正确，
/// 如果不是，包含判断会错误，因为牵涉onTouchMoved的修改，所以再没实际需求前，暂不处理
bool TablePageView::onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event *pEvent)
{
    if(_tablePagesPos.empty())
        return false;
    
    _pSelCellInfo = NULL;
    _lastPoint = pTouch->getLocation();
    //
    Size clippingSize = _pClippingNode->getContentSize();
    Rect rect(_position.x, _position.y, clippingSize.width, clippingSize.height);
    if(rect.containsPoint(_lastPoint))//ClippingNode 区域转换到父节点区域后判断
    {
        _isScrollEvent = true;
        _firstPoint = _lastPoint;
        _moveDirection = Direction::NONE;
        // 转换触摸点到ClippingNode内坐标
        Vec2 pos(_lastPoint.x - rect.getMinX(), _lastPoint.y - rect.getMinY());
        
        TablePage* pTPage = _tablePagesPos[_showPageInfo._showPageIndex].second;
        const Vec2& scrollPos = pTPage->_scrollPos[pTPage->_curPage];
        // 进一步转换坐标点到当前TablePage内坐标
        Vec2 offset(fabs(pos.x - scrollPos.x), pos.y + fabs(scrollPos.y));
        
        _pSelCellInfo = getCellPosInfoByPos(offset);
    }
    else
        _isScrollEvent = false;
    
    return true;
}

void TablePageView::onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event *pEvent)
{
    if(!_isScrollEvent)
        return;
    //
    if(_pSelCellInfo)
        _pSelCellInfo = NULL;
    //
    auto movePoint = pTouch->getLocation();
    Vec2 diff = movePoint - _lastPoint;
    // 只能一个方向移动
    float distance = movePoint.x - _firstPoint.x;
    if(((fabs(movePoint.y - _firstPoint.y) / fabs(distance) > 0.7) && _moveDirection != Direction::HORIZONTAL)
       || _moveDirection == Direction::VERTICAL)
    {
        _moveDirection = Direction::VERTICAL;
        diff.x = 0;
    }
    else if(_moveDirection != Direction::VERTICAL)
    {
        _moveDirection = Direction::HORIZONTAL;
        diff.y = 0;
    }
    else
    {
        CCASSERT(false, "异常");
    }
    //
    cocos2d::Node* pLayer = _tablePagesPos[_showPageInfo._showPageIndex].first->_pLayer;
    Vec2 offset = pLayer->getPosition() + diff;
    //
    offset.x = MAX(_showPageInfo.minOffset.x, MIN(_showPageInfo.maxOffset.x, offset.x));
    offset.y = MAX(_showPageInfo.minOffset.y, MIN(_showPageInfo.maxOffset.y, offset.y));
    //
    pLayer->setPosition(offset);
    
    TablePage* pTablePage = _tablePagesPos[_showPageInfo._showPageIndex].second;
    pTablePage->_scrollPos[pTablePage->_curPage] = offset;
    //
    _lastPoint = movePoint;
    //
    didScroll();
    //
    if(_pScrollBar)
        _pScrollBar->setPos(offset);
}

void TablePageView::onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event *pEvent)
{
    if(_pSelCellInfo)
    {
        CCLOG("x:%i,Y:%i", _pSelCellInfo->_row, _pSelCellInfo->_col);
        _pDelegate->cellTouched(_tablePagesPos[_showPageInfo._showPageIndex].second, _pSelCellInfo->_pCell);
        _pSelCellInfo = NULL;
        
        return;
    }
    //
    cocos2d::Node* pLayer = _tablePagesPos[_showPageInfo._showPageIndex].first->_pLayer;
    Vec2 offset = pLayer->getPosition();
    float right = offset.x + pLayer->getContentSize().width;
    
    float halfWidth = _pClippingNode->getContentSize().width / 2;
    if(right < halfWidth)
        goToPage(PMD_Left);
    else if(offset.x > halfWidth)
        goToPage(PMD_Right);
    else if(fabs(offset.x) > 1)
        pLayer->runAction(MoveTo::create(0.1, Vec2(0, offset.y)));
    //    else
    //    {
    //        CCASSERT(_pSelCellInfo, "选择单元格异常");
    //    }
}

SameDataTypeTablePage* TablePageView::findSameDataType(const int dataType)
{
    std::vector<SameDataTypeTablePage*>::iterator begin = _tablePages.begin();
    std::vector<SameDataTypeTablePage*>::iterator end = _tablePages.end();
    for(; begin != end; begin++)
    {
        if(dataType == (*begin)->_dataType)
            return (*begin);
    }
    
    return NULL;
}

CellPosInfo* TablePageView::getCellPosInfoByPos(const cocos2d::Vec2& offset)
{
    CellPosInfo* pCellInfo = NULL;
    
    ssize_t len = _cellPos.size();
    for(int i = 0; i < len; i++)
    {
        pCellInfo = &_cellPos[i];
        if(pCellInfo->_left <= offset.x && pCellInfo->_right >= offset.x &&
           pCellInfo->_bottom <= offset.y && pCellInfo->_top >= offset.y)
        {
            return pCellInfo;
        }
    }
    
    return NULL;
}

void TablePageView::updateShowPageInfo(int pageIndex)
{
    ssize_t len = _tablePagesPos.size();
    if((pageIndex < 0) || (pageIndex >= len))
    {
        CCASSERT(false, "页数索引异常！！！");
        return;
    }
    
    TablePage* pTPage = _tablePagesPos[pageIndex].second;
    //
    _showPageInfo._showPageIndex = pageIndex;
    _showPageInfo._curPage = pTPage->_curPage;
    // 左
    if(pTPage->_curPage > 0)
    {
        _showPageInfo._lShowPageIndex = pageIndex;
        _showPageInfo._lCurPage = pTPage->_curPage - 1;
    }
    else if(pageIndex > 0)
    {
        _showPageInfo._lShowPageIndex = pageIndex - 1;
        _showPageInfo._lCurPage = _tablePagesPos[pageIndex - 1].second->_curPage;// 需要切换到最后一页么？？？
    }
    else
    {
        _showPageInfo._lShowPageIndex = INVALID_SCROLL_POS;
        _showPageInfo._lCurPage = INVALID_SCROLL_POS;
    }
    // 右
    if(_pDelegate->numberOfPage(pTPage) - 1 > pTPage->_curPage)
    {
        _showPageInfo._rShowPageIndex = pageIndex;
        _showPageInfo._rCurPage = pTPage->_curPage + 1;
    }
    else if((_tablePagesPos.size() - 1) > pageIndex)
    {
        _showPageInfo._rShowPageIndex = pageIndex + 1;
        _showPageInfo._rCurPage = _tablePagesPos[pageIndex + 1].second->_curPage;// 需要切换到第一页么？？？
    }
    else
    {
        _showPageInfo._rShowPageIndex = INVALID_SCROLL_POS;
        _showPageInfo._rCurPage = INVALID_SCROLL_POS;
    }
    //
    cocos2d::Node* pLayer = _tablePagesPos[pageIndex].first->_pLayer;
    Size clippingSize = _pClippingNode->getContentSize();
    _showPageInfo.minOffset = clippingSize - pLayer->getContentSize();
    _showPageInfo.maxOffset.set(0, 0);
    
    if(!(_showPageInfo._lShowPageIndex < 0 && _showPageInfo._lCurPage < 0))
        _showPageInfo.maxOffset.x = _showPageInfo.maxOffset.x + clippingSize.width;
    if(!(_showPageInfo._rShowPageIndex < 0 && _showPageInfo._rCurPage < 0))
        _showPageInfo.minOffset.x = _showPageInfo.minOffset.x - clippingSize.width;
    //
}

void TablePageView::goToPage(PageMoveDirection direction)
{
    onPageSwitch();
    //
    int index = -1;
    int curPage = -1;
    switch (direction)
    {
        case PMD_Left:// 下一页
        {
            index = _showPageInfo._rShowPageIndex;
            curPage = _showPageInfo._rCurPage;
        }
            break;
        case PMD_Right:// 上一页
        {
            index = _showPageInfo._lShowPageIndex;
            curPage = _showPageInfo._lCurPage;
        }
            break;
        default:
            CCASSERT(false, "错误移动方向");
    }
    
    // 如果在同一个tablePages里面，那么移动到上一页或下一页
    if(index == _showPageInfo._showPageIndex)
        _tablePagesPos[index].second->_curPage = curPage;
    else// 如果不在同一个tablePages里面，那么切换到新tablePages，显示新tablePages最后显示的页面
        setHeaderSelected(index);
    
    _cellAniInfo._hasAni = true;
    _cellAniInfo._direction = direction;
    //
    showPage(index);
    //
    if(_cellAniInfo._pos.empty())
        CCASSERT(false, "单元格动画数据异常");
    else
        schedule(schedule_selector(TablePageView::updateCellAni));
}

void TablePageView::updateInfoLayer()
{
    Label* pLeftL = NULL;
    Label* pRightL = NULL;
    
    if(!_pInfoLayer)
    {
        float showWidth = getContentSize().width;
        float showHeight = 30;
        
        _pInfoLayer = LayerColor::create(Color4B(0, 0, 0, 130));
        _pInfoLayer->setContentSize(Size(showWidth, showHeight));
        
        pLeftL = Label::create();
        pLeftL->setTag(123);
        pLeftL->setSystemFontSize(20);
        pLeftL->ignoreAnchorPointForPosition(true);
        pLeftL->setDimensions(showWidth, showHeight);
        pLeftL->setAlignment(TextHAlignment::LEFT, TextVAlignment::CENTER);
        _pInfoLayer->addChild(pLeftL);
        
        pRightL = Label::create();
        pRightL->setTag(321);
        pRightL->setSystemFontSize(20);
        pRightL->ignoreAnchorPointForPosition(true);
        pRightL->setDimensions(showWidth, showHeight);
        pRightL->setAlignment(TextHAlignment::RIGHT, TextVAlignment::CENTER);
        _pInfoLayer->addChild(pRightL);
        
        addChild(_pInfoLayer);
    }
    
    pLeftL = dynamic_cast<Label*>(_pInfoLayer->getChildByTag(123));
    pRightL = dynamic_cast<Label*>(_pInfoLayer->getChildByTag(321));
    
    TablePage* pTPage = _tablePagesPos[_showPageInfo._showPageIndex].second;
    char showText[16] = {};
    sprintf(showText, " %i/%i", pTPage->_curPage + 1, _pDelegate->numberOfPage(pTPage));
    pLeftL->setString(showText);
    pRightL->setString(pTPage->_info._tipInfo);
}

void TablePageView::updateLRP()
{
    float halfHeight = getContentSize().height / 2;
    if(!_pLP)
    {
        _pLP = Sprite::create("lr.png", Rect(0, 0, 32, 32));
        _pLP->setAnchorPoint(Vec2(1, 0.5f));
        _pLP->setPosition(0, halfHeight);
        addChild(_pLP);
    }
    if(!_pRP)
    {
        _pRP = Sprite::create("lr.png", Rect(0, 0, 32, 32));
        _pRP->setFlippedX(true);
        _pRP->setAnchorPoint(Vec2(0, 0.5f));
        _pRP->setPosition(getContentSize().width, halfHeight);
        addChild(_pRP);
    }
    
    if(_showPageInfo._rShowPageIndex < 0 && _showPageInfo._rCurPage < 0)
        _pLP->setVisible(false);//_pLP->setTextureRect(Rect(32, 0, 32, 32));
    else
        _pLP->setVisible(true);//_pLP->setTextureRect(Rect(0, 0, 32, 32));
    
    if(_showPageInfo._lShowPageIndex < 0 && _showPageInfo._lCurPage < 0)
        _pRP->setVisible(false);//_pRP->setTextureRect(Rect(32, 0, 32, 32));
    else
        _pRP->setVisible(true);//_pRP->setTextureRect(Rect(0, 0, 32, 32));
}

void TablePageView::updateCellAni(float dt)
{
    if(!_cellAniInfo._hasAni)
    {
        CCASSERT(false, "处理单元格动画异常");
        return;
    }
    
    size_t rowNum = _cellAniInfo._pos.size();
    if(rowNum <= 0)
    {
        CCASSERT(false, "处理单元格动画异常");
        return;
    }
    
    if(_cellAniInfo._lastRunRow <= 0)
        _cellAniInfo._lastRunRow = 1;
    
    float tenth = _pClippingNode->getContentSize().width / 10;
    float leftShowLine = 0;//tenth * 2;
    float rightShowLine = tenth * 8;
    int moveStep = 30;
    size_t colNum = 0;
    bool rowOk = true;
    CellAniInfo::PosInfo* pPosInfo = NULL;
    std::vector<std::vector<CellAniInfo::PosInfo>>& posInfo = _cellAniInfo._pos;
    for(int i = _cellAniInfo._okRowNum; i < _cellAniInfo._lastRunRow; i++)
    {
        colNum = posInfo[i].size();
        for(int j = 0; j < colNum; j++)
        {
            pPosInfo = &posInfo[i][j];
            if(_cellAniInfo._direction == PMD_Left)
            {
                pPosInfo->_x -= moveStep;
                if(pPosInfo->_x < pPosInfo->_pCellPosInfo->_left)
                    pPosInfo->_x = pPosInfo->_pCellPosInfo->_left;
                else
                    rowOk = false;
            }
            else if(_cellAniInfo._direction == PMD_Right)// 这个需要完善，移动检测有问题
            {
                pPosInfo->_x += moveStep;
                if(pPosInfo->_x > pPosInfo->_pCellPosInfo->_left)
                    pPosInfo->_x = pPosInfo->_pCellPosInfo->_left;
                else
                    rowOk = false;
            }
            else
            {
                CCASSERT(false, "单元格移动异常");
            }
            if(pPosInfo->_pCellPosInfo->_isShow)
                pPosInfo->_pCellPosInfo->_pCell->setPositionX(pPosInfo->_x);
        }// for(int j
        if(rowOk)
            _cellAniInfo._okRowNum++;
    }
    
    if(_cellAniInfo._okRowNum >= rowNum)
    {
        _cellAniInfo._hasAni = false;
        _cellAniInfo._okRowNum = 0;
        _cellAniInfo._lastRunRow = 0;
        
        unschedule(schedule_selector(TablePageView::updateCellAni));
    }else// 判断是否启动下一行
    {
        pPosInfo = &posInfo[_cellAniInfo._lastRunRow - 1][0];
        if(_cellAniInfo._direction == PMD_Left)
        {
            if(pPosInfo->_x <= rightShowLine)
                _cellAniInfo._lastRunRow++;
        }
        else if(_cellAniInfo._direction == PMD_Right)
        {
            if(pPosInfo->_x >= leftShowLine)
                _cellAniInfo._lastRunRow++;
        }
        else
        {
            CCASSERT(false, "单元格移动异常");
        }
        
        if(_cellAniInfo._lastRunRow >= rowNum)
            _cellAniInfo._lastRunRow = rowNum;
    }
}

void TablePageView::updateScrollBar()
{
    const Size& contentSize = _tablePagesPos[_showPageInfo._showPageIndex].first->_pLayer->getContentSize();
    const Size& showSize = _pClippingNode->getContentSize();
    if(contentSize.height / showSize.height < 1.001f)
    {
        if(_pScrollBar)
            _pScrollBar->setVisible(false);
        
        return;
    }
    
    if(!_pScrollBar)
    {
        _pScrollBar = ScrollBar::create(showSize, contentSize);
        _pScrollBar->setPosition(showSize.width - _pScrollBar->getBarSize().width, 0);
        addChild(_pScrollBar);
    }
    else
        _pScrollBar->updateWithSize(showSize, contentSize);
    
    if(!_pScrollBar->isVisible())
        _pScrollBar->setVisible(true);
    
    TablePage* pTablePage = _tablePagesPos[_showPageInfo._showPageIndex].second;
    _pScrollBar->setPos(pTablePage->_scrollPos[pTablePage->_curPage]);
}

void TablePageView::AdjustHeader()
{
    size_t len = _tablePagesPos.size();
    float totalWidth = 0;
    int i = 0;
    for(; i < len; i++)
    {
        if(_tablePagesPos[i].second->_pHead)
            totalWidth += _tablePagesPos[i].second->_pHead->getContentSize().width;
    }
    
    float step = -1;
    if(totalWidth > getContentSize().width)
        step = getContentSize().width / len;
    
    float curPos = 0;
    for(int i = 0; i < len; i++)
    {
        if(_tablePagesPos[i].second->_pHead)
        {
            _tablePagesPos[i].second->_pHead->setPositionX(curPos);
            if(step > -1)
                curPos += step;
            else
                curPos += _tablePagesPos[i].second->_pHead->getContentSize().width;
        }
    }
}

void TablePageView::setHeaderSelected(int pageIndex)
{
    if((pageIndex < 0) || (pageIndex >= _tablePagesPos.size()))
    {
        CCASSERT(false, "页数索引异常！！！");
        return;
    }
    
    if(pageIndex == _showPageInfo._showPageIndex)
        return;
    
    if(_tablePagesPos[_showPageInfo._showPageIndex].second->_pHead)
        _tablePagesPos[_showPageInfo._showPageIndex].second->_pHead->setSelected(false);
    if(_tablePagesPos[pageIndex].second->_pHead)
        _tablePagesPos[pageIndex].second->_pHead->setSelected(true);
    
    // 在点table标签做事件处理和执行goToPage的时候，都会调用这个函数，所以下面这几句放这
    _tablePagesPos[_showPageInfo._showPageIndex].first->_pLayer->setVisible(false);
    //
    onPageSwitch();// 放这个函数在这，是否这个函数的名字setHeaderSelected就不太恰当？？？
}

void TablePageView::onPageSwitch()
{
    if(_cellAniInfo._hasAni)
    {
        unschedule(schedule_selector(TablePageView::updateCellAni));// 如果有动画正在做，停止？？？这个可以不用的
        _cellAniInfo._hasAni = false;
        _cellAniInfo._okRowNum = 0;
        _cellAniInfo._lastRunRow = 0;
        _cellAniInfo._direction = PMD_None;
        // _cellAniInfo._pos.clear();
    }
    
    SameDataTypeTablePage* pSameData = _tablePagesPos[_showPageInfo._showPageIndex].first;
    // CCLOG("use:%i, free:%i", pSameData->_cellsUsed.size(), pSameData->_cellsFreed.size());
    if(!pSameData->_cellsUsed.empty())
    {
        TablePageCell* pCell = NULL;
        int len = pSameData->_cellsUsed.size();
        for(int i = 0; i < len; i++)
        {
            pCell = pSameData->_cellsUsed[i];
            pCell->retain();// 对应dequeueCell里的autorelease？？？
            pCell->removeFromParentAndCleanup(true);
            
            pSameData->_cellsFreed.push_back(pCell);
        }
        pSameData->_cellsUsed.clear();
    }
    //
    // _cellPos.clear();
    // 归位层(有拖动层的操作)
    _tablePagesPos[_showPageInfo._showPageIndex].first->_pLayer->setPositionX(0.f);
}

//
TablePageView* TablePageView::create(TablePageDelegate* pDelegate, cocos2d::Size size, int pageHeadHeight)
{
    TablePageView *ret = new TablePageView(pDelegate);
    if (ret && ret->initWithSize(size, pageHeadHeight))
    {
        ret->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(ret);
    }
    
    return ret;
}

//PageMoveDirection TablePageView::getPageMoveDirection()
//{
//    if(_tablePagesPos.empty())
//        return PageMoveDirection::PMD_None;
//
//    unsigned int retVal = PageMoveDirection::PMD_Left | PageMoveDirection::PMD_Right;
//
//    TablePage* pTPage = _tablePagesPos[_showPageIndex].second;
//    if((0 == _showPageIndex) && (0 == pTPage->_curPage))
//        retVal = retVal ^ PMD_Left;
//
//    if(((_tablePagesPos.size() - 1) == _showPageIndex) && ((_pDelegate->numberOfPage(pTPage) - 1) == pTPage->_curPage))
//        retVal = retVal ^ PMD_Right;
//
//    // CCASSERT(false, "异常");
//
//    return static_cast<PageMoveDirection>(retVal);
//}

//void TablePageView::onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event *pEvent)
//{
//    Size clippingSize = _pClippingNode->getContentSize();
//    Vec2 minOffset = clippingSize - pLayer->getContentSize();
//    Vec2 maxOffset;
//
//    unsigned int moveDirection = getPageMoveDirection();// 这个可以优化，只调用一次，而不用在移动时不断调用
//    if(moveDirection & PageMoveDirection::PMD_Left)
//        maxOffset.x = maxOffset.x + clippingSize.width;
//    if(moveDirection & PageMoveDirection::PMD_Right)
//        minOffset.x = minOffset.x - clippingSize.width;
//}
