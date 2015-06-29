//
//  TablePageView.h
//
//  Created by 孙建华 on 2015.
//
//

#ifndef __Spine__TablePageView__
#define __Spine__TablePageView__

#include "cocos2d.h"
#include "TablePageHeader.h"

class TablePageCell;
class TablePageDelegate;
class ScrollBar;

//struct TablePageHeadInfo
//{
//    char _highlightImage[32];
//    char _unhighlightImage[32];
//    char _title[32];
//};
//
//struct TablePageHead
//{
//    cocos2d::Sprite* _pHighlight;
//    cocos2d::Sprite* _pUnhighlight;
//    // text
//};

struct TablePageInfo
{
    int _id;
    int _dataType;  // 相同数据类型，会被归类，单元格共享处理
    int _rowNum;    // 一页显示的最大行数
    int _colNum;    // 一页显示的最大列数
    int _leftMargin;
    int _topMargin;
    // tableView 是动态传递计算单元格大小，目前没用到大小不一单元格，所以在这里固定尺寸，应该是动态的，还有一个情况：单元格折叠
    int _cellWidth;
    int _cellHeight;
    int _cellVSpacing;
    int _cellHSpacing;
    
    char _tipInfo[64];
    //    float _tableCellOffsetX;
    //    float _tableCellOffsetY;
    
    TablePageHeadInfo _headInfo;
    
    // cocos2d::Node* _pAdditionalNode;
};

/// 要做：这个处理成一个类，在这个类里计算page的宽高，实现一个子类：折叠页，可以动态改变页尺寸
struct TablePage
{
    TablePageHeader* _pHead;
    TablePageInfo _info;
    
    /// 说明：当前显示的页，这样切回到TablePage，可以直接显示切出去时显示的page？？？
    int _curPage;
    /// 说明：记录每页滚动的最后位置，水平滚动为X，垂直滚动为Y，这个可以做到切换到别的标签，切换回来还显示到上次浏览位置
    std::vector<cocos2d::Vec2> _scrollPos;
    // cocos2d::Size _curPageSize;
};

//
struct SameDataTypeTablePage
{
    int _dataType;
    cocos2d::Node* _pLayer;// 作为容器用
    std::vector<TablePage*> _pages;
    std::vector<TablePageCell*> _cellsUsed;
    std::vector<TablePageCell*> _cellsFreed;
};

struct CellPosInfo
{
    bool _isShow;// 是否显示 根据变动处理_pCell
    int _row;
    int _col;
    TablePageCell* _pCell;
    
    float _left;
    float _top;
    float _right;
    float _bottom;
};

struct ShowPageInfo
{
    // 这个目前是取tablePage入堆的顺序，如果有删除操作，可能就异常了，要重构？？？
    int _showPageIndex;
    int _curPage;
    cocos2d::Vec2 minOffset;
    cocos2d::Vec2 maxOffset;
    // 左页
    int _lShowPageIndex;
    int _lCurPage;
    // 右页
    int _rShowPageIndex;
    int _rCurPage;
};

typedef enum {
    PMD_None=0x00,
    PMD_Left=0x01,
    PMD_Right=0x02
} PageMoveDirection;// Range

struct CellAniInfo
{
    struct PosInfo
    {
        float _x;
        // float _y; // 只对多行的左右移动做处理，所以Y坐标基本用不到
        CellPosInfo* _pCellPosInfo;
    };
    
    bool _hasAni;
    int _okRowNum;// 第几行已经做完动画
    int _lastRunRow;// 第几行开始做动画
    
    PageMoveDirection _direction;
    std::vector<std::vector<PosInfo>> _pos;// 按行保存
    
    CellAniInfo(){
        _hasAni = false;
        _okRowNum = 0;
        _lastRunRow = 0;
    }
};

class TablePageView : public cocos2d::Node
{
public:
    enum class Direction
    {
        NONE = -1,
        HORIZONTAL = 0,
        VERTICAL,
    };
    
public:
    TablePageView(TablePageDelegate* pDelegate);
    virtual ~TablePageView();
    
    bool initWithSize(const cocos2d::Size& size, int pageHeadHeight);
    void addPage(const TablePageInfo& info);
    void showPage(int pageIndex);
    void setHeaderSelected(int pageIndex);
    // 更新当前显示页的单元格位置，比如点击单元格，单元格变大，显示额外信息，就需要调用这个
    // void updateCellPos();
    //
    size_t getTablePageNum(){return _tablePagesPos.size();}
    int getShowPageIndex(){return _showPageInfo._showPageIndex;}
    const TablePage* getTablePage(int index)
    {if(index < 0 || index >= _tablePagesPos.size()){return NULL;} return _tablePagesPos[index].second;}
    
    TablePageCell* dequeueCell();
    const std::vector<TablePageCell*>& getShowCell();
    
    // void setDirection(TablePageView::Direction direction){_direction = direction;}
    // TablePageView::Direction getDirection(){return _direction;}
    
    //
    bool onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event *pEvent);
    void onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event *pEvent);
    void onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event *pEvent);
    
    //
    static TablePageView* create(TablePageDelegate* pDelegate, cocos2d::Size size, int pageHeadHeight = 0);
    
protected:
    void didScroll();
    //
    SameDataTypeTablePage* findSameDataType(const int dataType);
    CellPosInfo* getCellPosInfoByPos(const cocos2d::Vec2& offset);
    // PageMoveDirection getPageMoveDirection();
    void updateShowPageInfo(int pageIndex);
    void goToPage(PageMoveDirection direction);
    void updateInfoLayer();
    void updateLRP();
    void updateCellAni(float dt);
    void updateScrollBar();
    void AdjustHeader();
    void onPageSwitch();
    
protected:
    std::vector<std::pair<SameDataTypeTablePage*, TablePage*>> _tablePagesPos;// 这是按增加的循序入栈
    std::vector<SameDataTypeTablePage*> _tablePages;
    std::vector<CellPosInfo> _cellPos;
    CellAniInfo _cellAniInfo;
    
    TablePageDelegate* _pDelegate;
    
    int _pageHeadHeight;
    ShowPageInfo _showPageInfo;
    //
    cocos2d::ClippingRectangleNode* _pClippingNode;
    //
    cocos2d::Vec2 _firstPoint;// 用来计算是左右移动还是上下移动
    Direction _moveDirection;
    
    cocos2d::Vec2 _lastPoint;
    bool _isScrollEvent;
    CellPosInfo* _pSelCellInfo;
    //
    cocos2d::LayerColor* _pInfoLayer;
    // 这两个应该模仿TablePageDelegate，这样会比较灵活的定义指示箭头的样式和状态
    cocos2d::Sprite* _pLP;
    cocos2d::Sprite* _pRP;
    
    //
    ScrollBar* _pScrollBar;
};

#endif /* defined(__Spine__TablePageView__) */
