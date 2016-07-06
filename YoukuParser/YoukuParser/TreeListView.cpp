#include"stdafx.h"


///////////////////////////////////////////////////////////////////////////////////////
//
//
// MACRO\defines and Helpers
//
//
///////////////////////////////////////////////////////////////////////////////////////

#define TREELIST_FONT_EXTRA_HEIGHT              7
#define TREELIST_FONT_TEXT_CELL_OFFSET          9
#define TREELIST_PRNTCTLSIZE(TreeDim)           TreeDim.X,TreeDim.Y,TreeDim.Width,TreeDim.Hight
#define TREELIST_MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define TREELIST_MAX(a, b)  (((a) > (b)) ? (a) : (b))

#define TREELIST_WM_EDIT_NODE                   (WM_USER + 100)
#define TREELIST_WM_EDIT_ENTER                  (WM_USER + 101)
//#define TREELIST_ELEMENTS_PER_INSTANCE          4
//#define TREELIST_PROP_VAL                       TEXT("TREELIST_PTR")

//#define TREELIST_DOUBLE_BUFFEING

//typedef HANDLE(CALLBACK* LPREMOVEPROP)(HWND, LPCTSTR);  // VC2010 issue





// Dictionary pointer that will hold the ref count and HWND for each instance of the control
// a pointer to the dictionary will be attached to the parent window of the control.
// This array will be updated with each instance and destroyed when the last control will be terminated.








///////////////////////////////////////////////////////////////////////////////////////
//
// Function : TreeList_Internal_CRCCreate
// Notes    : Return a 32-bit CRC of the contents of the buffer
//
///////////////////////////////////////////////////////////////////////////////////////

unsigned long CTreeListView::TreeList_Internal_CRCCreate(const void *buf,	unsigned long bufLen) {
	unsigned long   crc32;
	unsigned long   i;
	unsigned char   *byteBuf;


	// Accumulate crc32 for a buffer
	crc32 = 0 ^ 0xFFFFFFFF;
	byteBuf = (unsigned char*)buf;

	for (i = 0; i < bufLen; i++) {
		crc32 = (crc32 >> 8) ^ TreeListCRC32Table[(crc32 ^ byteBuf[i]) & 0xFF];
	}

	return(crc32 ^ 0xFFFFFFFF);

}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function : TreeList_Internal_CRCCheck
// Notes    : Validates a crc32
// Returns  : 0 - Error
//            1 - Successes
//
///////////////////////////////////////////////////////////////////////////////////////

int CTreeListView::TreeList_Internal_CRCCheck(const void *buf,	unsigned long bufLen,unsigned long crc32) {
	if (TreeList_Internal_CRCCreate(buf, bufLen) == crc32) {
		return 1;
	}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////
//
// Function : TreeList_Internal_DestroyEditBox
// Notes    : Kill and free resources related to an edit box
// Returns  :
//
//
///////////////////////////////////////////////////////////////////////////////////////

void CTreeListView::TreeList_Internal_DestroyEditBox() {
	
				// Close edit box, when moving and resizing items
	if (HwndEditBox) {

		//TreeList_Internal_DictUpdate(true, this, nullptr, HwndEditBox);
		SetWindowLongPtr(HwndEditBox, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(ProcEdit)); // Restore the original wnd proc to the parent
		DestroyWindow(HwndEditBox);
		HwndEditBox = 0;
		EditedTreeItem = 0;
		EditedColumn = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function : TreeList_Internal_NodeGetLastSibling
// Notes    : Gets the last sibling of a given node
// Returns  :
//
//
///////////////////////////////////////////////////////////////////////////////////////

TreeListNode* CTreeListView::TreeList_Internal_NodeCreateNew() {
	TreeListNode    *pTmpNode = 0;

	pTmpNode = new TreeListNode;
	if (pTmpNode) {
		std::memset(pTmpNode, 0, sizeof(TreeListNode));
		AllocatedTreeBytes += sizeof(TreeListNode);
		pTmpNode->pNodeData.resize(ColumnsCount);
	}
	return pTmpNode;

}


///////////////////////////////////////////////////////////////////////////////////////
//
// Function : TreeList_Internal_NodeGetLastBrother
// Notes    : Gets the last brother of a given node
// Returns  :
//
//
///////////////////////////////////////////////////////////////////////////////////////

TreeListNode* CTreeListView::TreeList_Internal_NodeGetLastBrother(TreeListNode *pNode) {

	TreeListNode *pTmpNode = pNode;

	if (!pNode)
		return nullptr; // Invalid node as passed in

	if (pTmpNode->pBrother) {
		while (pTmpNode->pBrother)
			pTmpNode = pTmpNode->pBrother;
		return pTmpNode;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function : TreeList_Internal_NodeColonize
// Notes    : Updates a given node with the requested data
// Returns  :
//
//
///////////////////////////////////////////////////////////////////////////////////////

TreeListNode* CTreeListView::TreeList_Internal_NodeColonize(TreeListNode *pNode, TreeListNodeData *pNodeData) {

	if (!pNode)
		return nullptr; // No Node

	if (pNode->pNodeData.empty())
		return nullptr; // Null pointer

				  // Allocate the new element
	pNode->pNodeData[pNode->NodeDataCount] = new TreeListNodeData; // Allocate memory
	if (!pNode->pNodeData[pNode->NodeDataCount])
		return nullptr; // No memory

	AllocatedTreeBytes += sizeof(TreeListNodeData);

	memcpy(pNode->pNodeData[pNode->NodeDataCount], pNodeData, sizeof(TreeListNodeData)); // copy to the internal container
	pNode->pNodeData[pNode->NodeDataCount]->CRC = TreeList_Internal_CRCCreate(pNode->pNodeData[pNode->NodeDataCount],
		(sizeof(TreeListNodeData) - sizeof(pNode->pNodeData[pNode->NodeDataCount]->CRC))); // Set the crc sig
	pNode->NodeDataCount++;

	return pNode;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function : TreeList_Internal_AddNode
// Notes    : Add a nodeto the tree
// Returns  :
//
//
///////////////////////////////////////////////////////////////////////////////////////

TreeListNode* CTreeListView::TreeList_Internal_AddNode(TreeListNode *pParent) {

	TreeListNode *pNewNode = nullptr;
	TreeListNode *pLastBrother = nullptr;
	int           Nodes = 0;

	
				  // Special Case: Create the root node
	if (!pParent && !pRootNode) {
		pRootNode = TreeList_Internal_NodeCreateNew();
		return pRootNode;
	}

	// Special Case: Error, no root, no parent
	if (!pParent && !pRootNode)
		return nullptr;


	// Special Case: a root node brother
	if (!pParent) {
		pNewNode = TreeList_Internal_NodeCreateNew();
		if (pNewNode) {
			// Be the last brother to the root node (a new root node)
			pLastBrother = TreeList_Internal_NodeGetLastBrother(pRootNode);
			if (pLastBrother)
				pLastBrother->pBrother = pNewNode;
			else
				pRootNode->pBrother = pNewNode;

		}

		return pNewNode;
	}

	// Normal cases where there is a root node and we got the parent

	// Validate the parent integrity (NodeData crc)
	for (Nodes = 0;Nodes < pParent->NodeDataCount;Nodes++) {
		if (pParent->pNodeData[Nodes]) {
			if (!TreeList_Internal_CRCCheck(pParent->pNodeData[Nodes], (sizeof(TreeListNodeData) - sizeof(pParent->pNodeData[Nodes]->CRC)), pParent->pNodeData[Nodes]->CRC))
				return 0;
		}

	}
	// Be the last brother of our parent's siblig
	pNewNode = TreeList_Internal_NodeCreateNew();
	if (pNewNode) {
		pNewNode->pParennt = pParent;
		if (pParent->pSibling) // Our parent has a sibling?
		{
			pLastBrother = TreeList_Internal_NodeGetLastBrother(pParent->pSibling); // Are there any brothers to this sibling?
			if (pLastBrother)
				pLastBrother->pBrother = pNewNode; // There are brotheres, be the last of them
			else
				pParent->pSibling->pBrother = pNewNode; // There are no brother's, be the first
		} else
			pParent->pSibling = pNewNode; // Be the first sibling of our parent
	}

	return pNewNode;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function : TreeList_Internal_NodeFreeAllSubNodes
// Notes    : >> Recursive!! << Free all the nodes linked to a given node
// Returns  :
//
//
///////////////////////////////////////////////////////////////////////////////////////

void CTreeListView::TreeList_Internal_NodeFreeAllSubNodes(TreeListNode *pNode) {


	TreeListNode *pTmpNode = 0;
	int          Node;

	if (!pNode)
		return; // No session

	if (pNode->pSibling) {
		pTmpNode = pNode->pSibling;
		TreeList_Internal_NodeFreeAllSubNodes(pTmpNode);
	}

	if (pNode->pBrother) {
		pTmpNode = pNode->pBrother;
		TreeList_Internal_NodeFreeAllSubNodes(pTmpNode);

	}
	// Free the Nodes array
	for (Node = 0;Node < pNode->NodeDataCount;Node++) {
		if (pNode->pNodeData[Node]) {
			delete pNode->pNodeData[Node];
			pNode->pNodeData[Node] = nullptr;
			AllocatedTreeBytes -= sizeof(TreeListNodeData);
		}
	}

	delete pNode;
	pNode = nullptr;
	AllocatedTreeBytes -= sizeof(TreeListNode);
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function : TreeList_Internal_RectToDimensions
// Notes    : Convert MS rect to the internal TreeListDimensions struct
// Returns  :
//
//
///////////////////////////////////////////////////////////////////////////////////////

TreeListDimensions* CTreeListView::TreeList_Internal_RectToDimensions(RECT *pRect, TreeListDimensions *pDimensions) {

	if ((!pRect) || (!pDimensions))
		return nullptr;

	pDimensions->Width = pRect->right - pRect->left;
	pDimensions->Hight = pRect->bottom - pRect->top;
	pDimensions->X = pRect->left;
	pDimensions->Y = pRect->top;

	return pDimensions;

}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function : TreeList_Internal_AutoSetLastColumn
// Notes    : Last column to auto expand to the max width
// Returns  :
//
//
///////////////////////////////////////////////////////////////////////////////////////

void CTreeListView::TreeList_Internal_AutoSetLastColumn() {

	int     Width, NewWidth;
	int     StartPosition;

	if (ColumnsLocked == FALSE)
		return; // No session or that the headers are being updated


	ColumnDoAutoAdjust = FALSE;

	GetClientRect(HwndHeader, &RectHeader);

	// Get column width from the header control
	std::memset(&HeaderItem, 0, sizeof(HDITEM));
	HeaderItem.mask = HDI_WIDTH;

	ColumnsCount = Header_GetItemCount(HwndHeader);;
	if (ColumnsCount == -1)
		return;

	if (Header_GetItem(HwndHeader, ColumnsCount - 1, &HeaderItem) == TRUE) {
		Width = HeaderItem.cxy;
		StartPosition = ColumnsTotalWidth - Width;
		NewWidth = (RectHeader.right - RectHeader.left) - StartPosition + 2;

		std::memset(&HeaderItem, 0, sizeof(HDITEM));

		HeaderItem.mask = HDI_WIDTH;
		HeaderItem.cxy = NewWidth;
		ColumnsInfo[ColumnsCount - 1]->Width = NewWidth; // Store in session

		Header_SetItem(HwndHeader, ColumnsCount - 1, reinterpret_cast<LPARAM>(&HeaderItem));
	}

}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function : TreeList_Internal_UpdateColumns
// Notes    :
// Returns  :
//
//
///////////////////////////////////////////////////////////////////////////////////////

void CTreeListView::TreeList_Internal_UpdateColumns() {

	int     iCol;
	HDITEM  HeaderItem;
	RECT    RectHeaderItem, RectText;

	if (ColumnsLocked == FALSE)
		return; // No session or that the headers are being updated

	memset(&HeaderItem, 0, sizeof(HeaderItem));
	HeaderItem.mask = HDI_WIDTH;

	ColumnsCount = Header_GetItemCount(HwndHeader);
	if (ColumnsCount == -1)
		return;

	ColumnsTotalWidth = 0;

	// Get column widths from the header control
	for (iCol = 0; iCol < ColumnsCount; iCol++) {
		if (Header_GetItem(HwndHeader, iCol, &HeaderItem) == TRUE) {
			ColumnsInfo[iCol]->Width = HeaderItem.cxy;
			ColumnsTotalWidth += HeaderItem.cxy;

			if (iCol == 0)
				ColumnsFirstWidth = HeaderItem.cxy;
		}
	}

	// Resize the editbox
	if (HwndEditBox) {

		// Get the relevant sizes
		if (Header_GetItemRect(HwndHeader, EditedColumn, &RectHeaderItem) == FALSE)
			return;
		if (TreeView_GetItemRect(HwndTreeView, EditedTreeItem, &RectText, TRUE) == FALSE)
			return;

		SizeEdit.Width = RectHeaderItem.right - RectHeaderItem.left;
		SizeEdit.Hight = RectText.bottom - RectText.top;
		SizeEdit.X = RectHeaderItem.left;
		SizeEdit.Y = RectText.top;

		MoveWindow(HwndEditBox, SizeEdit.X + SizeRequested.X,                    // Position: left
			SizeEdit.Y + (RectHeaderItem.bottom - RectHeaderItem.top) + 1 + SizeRequested.Y,      // Position: top
			SizeEdit.Width - 1,    // Width
			SizeEdit.Hight - 2,    // Height
			true);
	}
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function : TreeList_Internal_RepositionControls
// Notes    : 
// Returns  :
//
//
///////////////////////////////////////////////////////////////////////////////////////
void CTreeListView::TreeList_Internal_RepositionControls() {

	DWORD dwStyle;
	int   iOffSet = 0;

				// Get Parrent Size
	GetClientRect(HwndParent, &RectParent);
	TreeList_Internal_RectToDimensions(&RectParent, &SizeParent);

	if (UseFullSize)
		memcpy(&SizeRequested, &SizeParent, sizeof(TreeListDimensions)); // Full size, the control size is the same as it's parent client size
	else // Get the size of the requested rect
		TreeList_Internal_RectToDimensions(&RectRequested, &SizeRequested);

	// Set Header Window
	memcpy(&SizeHeader, &SizeRequested, sizeof(TreeListDimensions));
	SizeHeader.Hight = FontInfoHeader.lfHeight + TREELIST_FONT_EXTRA_HEIGHT;

	if (GotAnchors) {
		if ((CreateFlags & TREELIST_ANCHOR_RIGHT) == TREELIST_ANCHOR_RIGHT)
			SizeHeader.Width = SizeParent.Width - PointAnchors.x - RectRequested.left;
	}

	// Set TreeList Window size
	memcpy(&SizeTree, &SizeRequested, sizeof(TreeListDimensions));
	SizeTree.Hight = (SizeTree.Hight - SizeHeader.Hight);
	SizeTree.Y += SizeHeader.Hight;

	// Use specified anchores
	if (GotAnchors) {

		if ((CreateFlags & TREELIST_ANCHOR_RIGHT) == TREELIST_ANCHOR_RIGHT)
			SizeTree.Width = SizeParent.Width - PointAnchors.x - RectRequested.left;

		if ((CreateFlags & TREELIST_ANCHOR_BOTTOM) == TREELIST_ANCHOR_BOTTOM)
			SizeTree.Hight = SizeParent.Hight - (PointAnchors.y + SizeHeader.Hight) - RectRequested.top;
	}

	// Move Windows
	MoveWindow(HwndTreeView, TREELIST_PRNTCTLSIZE(SizeTree), 1);
	MoveWindow(HwndHeader, TREELIST_PRNTCTLSIZE(SizeHeader), 1);

	// Get the current TreeView rect
	GetClientRect(HwndTreeView, &RectTree);

	// Get the current Header rect
	GetClientRect(HwndHeader, &RectHeader);

	// Get treelist position relatve to the parent
	memcpy(&RectClientOnParent, &RectTree, sizeof(RECT));
	MapWindowPoints(HwndTreeView, HwndParent, reinterpret_cast<LPPOINT>(&RectClientOnParent), 2);

	// Set the border rect, we should resize it as needed, so we have to map it to the parent size
	if ((CreateFlags & TREELIST_DRAW_EDGE) == TREELIST_DRAW_EDGE) {

		if (UseAnchors && GotAnchors) {
			InvalidateRect(HwndParent, &RectBorder, true); // Delete the prev border rect
		}

		memcpy(&RectBorder, &RectClientOnParent, sizeof(RECT));
		// Check if we have vertical scrollbars
		dwStyle = GetWindowLong(HwndTreeView, GWL_STYLE);
		if ((dwStyle & WS_VSCROLL) != 0)
			iOffSet = GetSystemMetrics(SM_CXVSCROLL);

		RectBorder.top = ((RectBorder.top - SizeHeader.Hight) - 5);
		RectBorder.left -= 2;
		RectBorder.right += 2 + iOffSet;
		RectBorder.bottom += 2;
		InvalidateRect(HwndParent, &RectBorder, false);
	}

	// Anchors?
	if (UseAnchors) {
		if (!GotAnchors) {
			PointAnchors.x = RectParent.right - RectClientOnParent.right;
			PointAnchors.y = RectParent.bottom - RectClientOnParent.bottom;
			GotAnchors = true;
		}
	}

	TreeList_Internal_DestroyEditBox();         // Kill the edit box, when moving and resizing items
	TreeList_Internal_UpdateColumns();          // Update the columns sizes
	TreeList_Internal_AutoSetLastColumn();      // Set the last col to be max size


}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function : TreeList_Internal_GetNodeFromTreeHandle
// Notes    : Returns the internal node pointer by the MS node handle
// Returns  :
//
//
///////////////////////////////////////////////////////////////////////////////////////

TreeListNode* CTreeListView::TreeList_Internal_GetNodeFromTreeHandle(HTREEITEM hTreeItem) {


	TVITEM              TreeItem;
	TreeListNode        *pNode = nullptr;
	int                 Node;


	memset(&TreeItem, 0, sizeof(TreeItem)); // Set all items to 0

	TreeItem.mask = TVIF_HANDLE;
	TreeItem.hItem = hTreeItem;

	if (TreeView_GetItem(HwndTreeView, &TreeItem) == false)
		return nullptr;

	pNode = reinterpret_cast<TreeListNode*>(TreeItem.lParam);

	// Check the node(s) integrity
	for (Node = 0;Node < pNode->NodeDataCount;Node++) {
		if (pNode->pNodeData[Node]) {
			if (!TreeList_Internal_CRCCheck(pNode->pNodeData[Node], (sizeof(TreeListNodeData) - sizeof(pNode->pNodeData[Node]->CRC)), pNode->pNodeData[Node]->CRC))
				return 0;
		}
	}

	return pNode;

}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function : TreeList_Internal_DeflateRect
// Notes    : See MFC DeflateRect it's the same
// Returns  :
//
//
///////////////////////////////////////////////////////////////////////////////////////

RECT* CTreeListView::TreeList_Internal_DeflateRect(RECT *pRect, int left, int top, int right, int bottom) {

	if (!pRect)
		return nullptr;

	pRect->bottom -= bottom;
	pRect->right -= right;
	pRect->top += top;
	pRect->left += left;

	return pRect;

}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function : TreeList_Internal_DeflateRectEx
// Notes    : See MFC DeflateRect it's the same
// Returns  :
//
//
///////////////////////////////////////////////////////////////////////////////////////

RECT* CTreeListView::TreeList_Internal_DeflateRectEx(RECT *pRect, int x, int y) {

	if (!pRect)
		return 0;

	return TreeList_Internal_DeflateRect(pRect, x, y, x, y);
}


////////////////////////////////////////////////////////////////////////////////////
//
// Function: Tree subclassing static interface functions
// Description: connect to three real non-static message handler functions
// Parameters:
// Return:
// Logic:
//
////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK CTreeListView::Static_TreeList_Internal_HandleEditBoxMessages(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	CTreeListView* current = reinterpret_cast<CTreeListView*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	assert(current);
	return current->TreeList_Internal_HandleEditBoxMessages(hWnd, Msg, wParam, lParam);
}
LRESULT CALLBACK CTreeListView::Static_TreeList_Internal_HandleTreeMessagesEx(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	CTreeListView* current = reinterpret_cast<CTreeListView*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	assert(current);
	return current->TreeList_Internal_HandleTreeMessagesEx(hWnd, Msg, wParam, lParam);
}
LRESULT CALLBACK CTreeListView::Static_TreeList_Internal_HandleTreeMessages(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	CTreeListView* current = reinterpret_cast<CTreeListView*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	assert(current);
	return current->TreeList_Internal_HandleTreeMessages(hWnd, Msg, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: TreeList_Internal_HandleEditBoxMessages
// Description:Subclassing the editbox to the get the key press notifications
// Parameters:
// Return:
// Logic:
//
////////////////////////////////////////////////////////////////////////////////////

LRESULT CTreeListView::TreeList_Internal_HandleEditBoxMessages(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {

	switch (Msg) {
	case WM_KEYUP: // Trap the enter key (this is needed when we are working with a dialog)
	{
		if (wParam == 0x0d) // Enter key was pressed
			PostMessage(HwndParent, TREELIST_WM_EDIT_ENTER, 0, reinterpret_cast<LPARAM>(this));
	}
	break;
	}

	return CallWindowProc(ProcEdit, hWnd, Msg, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: TreeList_Internal_HandleTreeMessagesEx
// Description:Subclassing the treelist to get the scrolling notifications
// Parameters:
// Return:
// Logic:
//
////////////////////////////////////////////////////////////////////////////////////

LRESULT CTreeListView::TreeList_Internal_HandleTreeMessagesEx(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {

	switch (Msg) {

		case WM_VSCROLL: // Trap the vertical scroll
		{

			TreeList_Internal_DestroyEditBox(); // Kill the edit box when the user scrolls vertically
		}
		break;

		case WM_LBUTTONUP:
		{
			ItemWasSelected = true;

		}
		break;
	}


	return CallWindowProc(ProcTreeList, hWnd, Msg, wParam, lParam);
}


////////////////////////////////////////////////////////////////////////////////////
//
// Function:    TreeList_Internal_HandleTreeMessages
// Description: The control main message handler
// Parameters:  Windows wndProc params
// Note:        The API will subclass the parent window and this function will handle all of its messages
//
// Return:      LRESULT: FALSE if a message was not handled
//
////////////////////////////////////////////////////////////////////////////////////

LRESULT CTreeListView::TreeList_Internal_HandleTreeMessages(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {

	LPNMTVCUSTOMDRAW    lpNMTVCustomDraw = 0;
	HTREEITEM           hTreeItem = 0;
	HDITEM              HeaderItem;
	LPNMHDR             lpNMHeader = 0;
	LPNMHEADER          lpNMHeaderChange = 0;
	TVHITTESTINFO       TreeTestInfo = { 0 };
	TVITEM              Treeitem = { 0 };
	WNDPROC             ProcParent = (WNDPROC)GetProp(hWnd, TEXT("WNDPROC"));
	RECT                RectLabel = { 0,0,0,0 };
	RECT                RectText = { 0,0,0,0 };
	RECT                RectItem = { 0,0,0,0 };
	RECT                RectHeaderItem = { 0,0,0,0 };
	RECT                RectParent = { 0,0,0,0 };
	HBRUSH              brWnd = 0;
	HBRUSH              brTextBk = 0;
	COLORREF            clTextBk = 0, clWnd = 0;
	HDC                 hDC = 0;
	HDC                 hDCMem = 0;
	HBITMAP             hBitMapMem = 0;
	HBITMAP             hOldBitMap = 0;
	LPNMHEADER          pNMHeader = 0;
	DWORD               dwMousePos = 0;
	DWORD               dwStyle = 0;
	bool                RetVal = false;
	bool                SelectedLine = false;
	int                 iCol = 0;
	int                 iOffSet = 0;
	int                 iColEnd, iColStart = 0;
	int                 iControlID = 0;
	TreeListNode        *pNode = 0;
	LPWINDOWPOS         lpPos = 0;


	switch (Msg) {



		///////////////////////////////////////////

	case WM_SIZE:{
		// Maximize
		if (wParam == SIZE_MAXIMIZED) {
			PostMessage(hWnd, WM_EXITSIZEMOVE, 0, 0);
			break;
		}
		// Restore down
			if (ParentResizing == true)
				break;
			PostMessage(hWnd, WM_EXITSIZEMOVE, 0, 0);
			break;

	}
	break;
	///////////////////////////////////////////


	case WM_ENTERSIZEMOVE: {
		ParentResizing = true;
	}
	break;
	///////////////////////////////////////////

	case WM_EXITSIZEMOVE: {
			ParentResizing = false;			
			// Check if we have to reposition the controls
			GetClientRect(HwndParent, &RectParent);
			if (memcmp(&RectParent, &RectParent, sizeof(RECT)) != 0) {
				if ((UseFullSize == true) || (UseAnchors == true)) {
					ItemWasSelected = false;
					TreeList_Internal_RepositionControls();
				}
			}
		}
	break;


	///////////////////////////////////////////

	case WM_PAINT:	{

			if (ColumnDoAutoAdjust == true)
				TreeList_Internal_AutoSetLastColumn();
		

		// Double Buffering, might kill the flickering
#ifdef TREELIST_DOUBLE_BUFFEING

		hDC = BeginPaint(hWnd, &pSession->PaintStruct);
		if (hDC) {
			RetVal = TRUE;
			hDCMem = CreateCompatibleDC(NULL);

			// Select the bitmap within the DC:
			hBitMapMem = CreateCompatibleBitmap(hDC, 800, 600);
			hOldBitMap = SelectObject(hDCMem, hBitMapMem);
			GetClientRect(pSession->HwndTreeView, &pSession->RectTree);
			BitBlt(hDC, 0, 0, pSession->RectTree.right, pSession->RectTree.bottom, hDCMem, 0, 0, SRCCOPY);
			SelectObject(hDCMem, hOldBitMap);

			DeleteObject(hBitMapMem);
			DeleteDC(hDCMem);
			EndPaint(hWnd, &pSession->PaintStruct);

		}
#endif

	}
	break;


	///////////////////////////////////////////
	case    WM_CTLCOLOREDIT:
	{
		if ((reinterpret_cast<HWND>(lParam) == HwndEditBox) && (EditBoxStyleNormal == false)) {
			RetVal = true;
			SetTextColor(reinterpret_cast<HDC>(wParam), RGB(255, 255, 255));
			SetBkColor(reinterpret_cast<HDC>(wParam), RGB(0, 0, 0));
		}
	}
	break;

	///////////////////////////////////////////

	case TREELIST_WM_EDIT_NODE:
	{
		if (ParentResizing)
			break;

		if (HwndEditBox)
			break;


		RetVal = true;
		pNode = TreeList_Internal_GetNodeFromTreeHandle(EditedTreeItem);
		if (pNode) {
			// This is place to edit a cell that needs editing
			if (Header_GetItemRect(HwndHeader, EditedColumn, &RectHeaderItem) == false) {
				EditedColumn = 0;
				EditedTreeItem = 0;
				break;
			}
			if (TreeView_GetItemRect(HwndTreeView, EditedTreeItem, &RectText, true) == false) {
				EditedColumn = 0;
				EditedTreeItem = 0;
				break;
			}
			SizeEdit.Width = RectHeaderItem.right - RectHeaderItem.left;
			SizeEdit.Hight = RectText.bottom - RectText.top;
			SizeEdit.X = RectHeaderItem.left;
			SizeEdit.Y = RectText.top;

			// Check if we have vertical scrollbars
			// Only if we are editing the last column (4/26/2011)
			iOffSet = 0;
			if (EditedColumn == (ColumnsCount - 1)) {
				dwStyle = GetWindowLong(HwndTreeView, GWL_STYLE);
				if ((dwStyle & WS_VSCROLL) != 0)
					iOffSet = GetSystemMetrics(SM_CXVSCROLL);
			}
			// a Numeric only text box
			if (pNode->pNodeData[EditedColumn]->Numeric == true)
				dwStyle = WS_CHILD | WS_VISIBLE | ES_NUMBER;
			else
				dwStyle = WS_CHILD | WS_VISIBLE;

			HwndEditBox = CreateWindowEx(0,
				TEXT("EDIT"),
				pNode->pNodeData[EditedColumn]->text.c_str(),
				dwStyle,
				SizeEdit.X + SizeRequested.X,                                                       // Position: left
				SizeEdit.Y + ((RectHeaderItem.bottom - RectHeaderItem.top)) + SizeRequested.Y,    // Position: top
				SizeEdit.Width - 1 - iOffSet,    // Width
				SizeEdit.Hight - 2,            // Height
				HwndParent,                  // Parent window handle
				0,
				InstanceParent, 0);

			if (HwndEditBox) {
				SetWindowLongPtr(HwndEditBox, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
				ProcEdit = reinterpret_cast<WNDPROC>(SetWindowLongPtr(HwndEditBox, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Static_TreeList_Internal_HandleEditBoxMessages))); // Subclassing the control (it will help trapping the enter key press event on a dialog)
				SendMessage(HwndEditBox, WM_SETFONT, reinterpret_cast<WPARAM>(FontHandleEdit), static_cast<LPARAM>(true));
				SetFocus(HwndEditBox);
				SendMessage(HwndEditBox, EM_SETSEL, pNode->pNodeData[EditedColumn]->text.size(), pNode->pNodeData[EditedColumn]->text.size()); // Select all text

			} else {
				EditedColumn = 0;
				EditedTreeItem = 0;
			}

		}

	}
	break;
	///////////////////////////////////////////
	case TREELIST_WM_EDIT_ENTER:
	{


		if (ParentResizing)
			break;

		if (WaitingForCaller == true)// Do nothong while waiting for the caller
			break;

		pNode = TreeList_Internal_GetNodeFromTreeHandle(EditedTreeItem);
		if (!pNode)
			break;

		RetVal = true;
		EditBoxBuffer.clear();
		EditBoxBuffer.resize(TREELIST_MAX_STRING + 1);
		if (GetWindowText(HwndEditBox, &EditBoxBuffer[0], TREELIST_MAX_STRING) > 0)// Get edit box text
		{
			if (EditBoxBuffer != pNode->pNodeData[EditedColumn]->text) {
				// Call the user!
				if (pCBValidateEdit) {
					WaitingForCaller = true;
					EditBoxOverrideBuffer.clear();

					if (pCBValidateEdit(pNode->pNodeData[EditedColumn]->pExternalPtr, const_cast<const tstring&>(EditBoxBuffer), EditBoxOverrideBuffer) == true) {
						if (!EditBoxOverrideBuffer.empty()) {
							pNode->pNodeData[EditedColumn]->text = EditBoxOverrideBuffer;
						} else {
							pNode->pNodeData[EditedColumn]->text = EditBoxBuffer;
						}
						pNode->pNodeData[EditedColumn]->Altered = true; // Set "changed" flag so we will be able to color it later
																				  // Update CRC
						pNode->pNodeData[EditedColumn]->CRC = TreeList_Internal_CRCCreate(pNode->pNodeData[EditedColumn], (sizeof(TreeListNodeData) - sizeof(pNode->pNodeData[EditedColumn]->CRC))); // Set the srs sig
					}
					WaitingForCaller = false;
				}
			}
		}


		TreeList_Internal_DestroyEditBox();// Kill the edit box

		// Select the edited line in the Tree
		if (pNode && EditedColumn > 0)
			TreeView_SelectItem(HwndTreeView, pNode->TreeItemHandle);

		SetFocus(HwndTreeView);
		EditedColumn = 0;
		EditedTreeItem = 0;
		break;

	}
	break;
	///////////////////////////////////////////
	case WM_NOTIFY:
	{


		lpNMHeader = (LPNMHDR)lParam;
		if (ParentResizing)
			break;

		switch (lpNMHeader->code) {


			///////////////////////////////////////////
		case NM_CUSTOMDRAW:
		{

			RetVal = true;
			lpNMTVCustomDraw = reinterpret_cast<LPNMTVCUSTOMDRAW>(lParam);

			if (lpNMHeader->hwndFrom != HwndTreeView)
				break;

			switch (lpNMTVCustomDraw->nmcd.dwDrawStage) {

			case CDDS_PREPAINT:
				return CDRF_NOTIFYITEMDRAW;

			case CDDS_ITEMPREPAINT:
				return (CDRF_DODEFAULT | CDRF_NOTIFYPOSTPAINT);

			case CDDS_ITEMPOSTPAINT:
			{


				// Get a valid pinter to our internal data type
				hTreeItem = reinterpret_cast<HTREEITEM>(lpNMTVCustomDraw->nmcd.dwItemSpec);
				pNode = TreeList_Internal_GetNodeFromTreeHandle(hTreeItem);
				if (!pNode || pNode->NodeDataCount == 0)
					return(CDRF_DODEFAULT);

				memcpy(&RectItem, &lpNMTVCustomDraw->nmcd.rc, sizeof(RECT));
				if (IsRectEmpty(&RectItem)) {
					// Nothing to paint when we have an empty rect
					SetWindowLongPtr(HwndParent, DWLP_MSGRESULT, CDRF_DODEFAULT);
					break;
				}

				hDC = lpNMTVCustomDraw->nmcd.hdc;
				if (!hDC)
					return(CDRF_DODEFAULT); // No HDC

				SetBkMode(hDC, TRANSPARENT);

				if (TreeView_GetItemRect(HwndTreeView, hTreeItem, &RectLabel, true) == false)
					return(CDRF_DODEFAULT); // No RECT


				SetTextColor(hDC, RGB(0, 0, 0)); // Make sure we use black color
				clTextBk = lpNMTVCustomDraw->clrTextBk;
				clWnd = TreeView_GetBkColor(HwndTreeView);
				brTextBk = CreateSolidBrush(clTextBk);
				brWnd = CreateSolidBrush(clWnd);

				// Clear the original label rectangle
				RectLabel.right = RectTree.right;
				FillRect(hDC, &RectLabel, brWnd);

				ColumnsCount = Header_GetItemCount(HwndHeader);
				if (ColumnsCount == -1)
					return(CDRF_DODEFAULT); // No columns info, nothing to do


											// Draw the horizontal lines
				for (iCol = 0; iCol < ColumnsCount; iCol++) {
					// Get current columns width from the header window
					memset(&HeaderItem, 0, sizeof(HeaderItem));
					HeaderItem.mask = HDI_HEIGHT | HDI_WIDTH;
					if (Header_GetItem(HwndHeader, iCol, &HeaderItem)) {

						ColumnsInfo[iCol]->Width = HeaderItem.cxy;
						iOffSet += HeaderItem.cxy;
						RectItem.right = iOffSet - 1;
						DrawEdge(hDC, &RectItem, BDR_SUNKENINNER, BF_RIGHT);
					}
				}

				// Draw the vertical lines
				DrawEdge(hDC, &RectItem, BDR_SUNKENINNER, BF_BOTTOM);

				// Draw Label, calculate the rect first
				if (pNode->pNodeData[0]->type == TEXT) {
					DrawText(hDC, pNode->pNodeData[0]->text.c_str(), static_cast<int>(pNode->pNodeData[0]->text.size()), &RectText, DT_NOPREFIX | DT_CALCRECT);
				} else if (pNode->pNodeData[0]->type == IMAGELIST) {

				} else if (pNode->pNodeData[0]->type == HWINDOW) {
					pNode->pNodeData[0]->pWindow->CMoveWindow(RectText.left, RectText.top, RectText.right - RectText.left, RectText.bottom - RectText.top, true);
					pNode->pNodeData[0]->pWindow->Show();
				}
				
				RectLabel.right = TREELIST_MIN((RectLabel.left + RectText.right + 4), ColumnsInfo[0]->Width - 4);

				if ((RectLabel.right - RectLabel.left) < 0)
					brTextBk = brWnd;

				if (clTextBk != clWnd)  // Draw label's background
				{

					if (ItemWasSelected == true) {
						SelectedLine = true;
						SetTextColor(hDC, RGB(255, 255, 255));
						RectLabel.right = RectTree.right;
						FillRect(hDC, &RectLabel, brTextBk);
					}
				}

				// Draw main label
				memcpy(&RectText, &RectLabel, sizeof(RECT));

				// The label right shoud be as the column right
				if (Header_GetItemRect(HwndHeader, 0, &RectHeaderItem) == false)
					return(CDRF_DODEFAULT);// Error getting the rect


				RectText.right = RectHeaderItem.right; // Set the right side
				TreeList_Internal_DeflateRectEx(&RectText, 2, 1); // Defalate it

				if (pNode->pNodeData[0]->type == TEXT) {
					DrawText(hDC, pNode->pNodeData[0]->text.c_str(), static_cast<int>(pNode->pNodeData[0]->text.size()), &RectText, DT_NOPREFIX | DT_END_ELLIPSIS);
				} else if (pNode->pNodeData[0]->type == IMAGELIST) {

				} else if (pNode->pNodeData[0]->type == HWINDOW) {
					pNode->pNodeData[0]->pWindow->CMoveWindow(RectText.left, RectText.top, RectText.right - RectText.left, RectText.bottom - RectText.top, true);
					pNode->pNodeData[0]->pWindow->Show();
				}

				iOffSet = ColumnsInfo[0]->Width;

				// Draw thwe other labels (the columns)
				for (iCol = 1; iCol < ColumnsCount; iCol++) {

					if (pNode->pNodeData[iCol]) {
						memcpy(&RectText, &RectLabel, sizeof(RECT));
						RectText.left = iOffSet;
						RectText.right = iOffSet + ColumnsInfo[iCol]->Width;

						// Set cell bk color
						if ((SelectedLine == false) && (pNode->pNodeData[iCol]->Colored == true)) {
							memcpy(&RectLabel, &RectText, sizeof(RECT));
							if (brTextBk)
								DeleteObject(brTextBk);
							brTextBk = CreateSolidBrush(pNode->pNodeData[iCol]->BackgroundColor);
							RectLabel.top += 1;
							RectLabel.bottom -= 1;
							RectLabel.right -= 2;
							FillRect(hDC, &RectLabel, brTextBk);

						}


						TreeList_Internal_DeflateRect(&RectText, TREELIST_FONT_TEXT_CELL_OFFSET, 1, 2, 1); // This is an "MFC" remake thing :)

						if (pNode->pNodeData[iCol]) {

							// Set specific text color (only when this is not the selected line)
							if (SelectedLine == false) {
								if (pNode->pNodeData[iCol]->Colored == true)
									SetTextColor(hDC, pNode->pNodeData[iCol]->TextColor);

								// Set special color for altered cells (if set by ty the caller)
								if (pNode->pNodeData[iCol]->Altered == true)
									SetTextColor(hDC, pNode->pNodeData[iCol]->AltertedTextColor);

							}

							if (pNode->pNodeData[iCol]->type == TEXT) {
								DrawText(hDC, pNode->pNodeData[iCol]->text.c_str(), static_cast<int>(pNode->pNodeData[iCol]->text.size()), &RectText, DT_NOPREFIX | DT_END_ELLIPSIS);
							} else if (pNode->pNodeData[iCol]->type == IMAGELIST) {

							} else if (pNode->pNodeData[iCol]->type == HWINDOW) {
								pNode->pNodeData[iCol]->pWindow->CMoveWindow(RectText.left, RectText.top, RectText.right - RectText.left, RectText.bottom - RectText.top, true);
								pNode->pNodeData[iCol]->pWindow->Show();
							}

						}
						iOffSet += ColumnsInfo[iCol]->Width;
					}
				}

				SetTextColor(hDC, RGB(0, 0, 0));

				// Draw the rect (on the parent) around the tree
				if ((CreateFlags & TREELIST_DRAW_EDGE) == TREELIST_DRAW_EDGE)
					DrawEdge(GetDC(HwndParent), &RectBorder, EDGE_ETCHED, BF_RECT);

				return(CDRF_DODEFAULT);
			}
			break;
			}
		}
		break;

		///////////////////////////////////////////
		case NM_RELEASEDCAPTURE:
		{


			RetVal = true;
			TreeList_Internal_DestroyEditBox(); // Kill the edit box, when resizing a column

			TreeList_Internal_UpdateColumns();
			ColumnDoAutoAdjust = true;

			// Refresh
			InvalidateRect(HwndParent, &RectClientOnParent, true);



		}
		break;

		///////////////////////////////////////////
		case NM_CLICK:
		{

			if (lpNMHeader->hwndFrom != HwndTreeView)
				break;

			if (WaitingForCaller == true)// Do nothong while waiting for the caller
				break;

			TreeList_Internal_DestroyEditBox();// Kill the edit box

			RetVal = false;  // Will make our control respond to click same as dblcick
			dwMousePos = GetMessagePos();
			TreeTestInfo.pt.x = GET_X_LPARAM(dwMousePos);
			TreeTestInfo.pt.y = GET_Y_LPARAM(dwMousePos);
			MapWindowPoints(HWND_DESKTOP, lpNMHeader->hwndFrom, &TreeTestInfo.pt, 1);
			TreeView_HitTest(lpNMHeader->hwndFrom, &TreeTestInfo);

			if (TreeTestInfo.hItem) {
				pNode = TreeList_Internal_GetNodeFromTreeHandle(TreeTestInfo.hItem);
				if (!pNode)
					break;

				ItemWasSelected = true;
				InvalidateRect(HwndTreeView, &RectTree, false);

				// Get the correct column where the mouse has clicked..
				iColStart = RectHeader.left;
				iColEnd = 0;
				for (iCol = 0;iCol < ColumnsCount; iCol++) {
					iColEnd += ColumnsInfo[iCol]->Width;

					if ((TreeTestInfo.pt.x >= iColStart) && (TreeTestInfo.pt.x < iColEnd)) {
						if (pNode->NodeDataCount >= iCol) // Is there any data there?
						{
							if (pNode->pNodeData[iCol] && pNode->pNodeData[iCol]->Editable == true) // Is it editable?
							{

								// Send edit message to the parent window
								EditedColumn = iCol;
								EditedTreeItem = TreeTestInfo.hItem;
								PostMessage(HwndParent, TREELIST_WM_EDIT_NODE, 0, reinterpret_cast<LPARAM>(this));
								break;
							}
						}
					}
					iColStart = iColEnd;
				}

				if (HwndEditBox)
					SendMessage(HwndParent, TREELIST_WM_EDIT_ENTER, 0, 0);
				else {
					if (iCol > 0)
						TreeView_SelectItem(lpNMHeader->hwndFrom, TreeTestInfo.hItem);
				}
			}
		}

		break;
		}
	}
	}

	if (hDC) // Cleanup
	{

		if (brWnd)
			DeleteObject(brWnd);

		if (brTextBk)
			DeleteObject(brTextBk);

	}

	if (RetVal == false) {
		if (ProcParent)
			return CallWindowProc(ProcParent, hWnd, Msg, wParam, lParam);
		else
			return false;
	} else
		return RetVal;

}

///////////////////////////////////////////////////////////////////////////////////////
//
//
// API
//
//
///////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
//
// Function:    TreeListCreate
// Description: Init the TreeList API, Allocates memory and create the internal data types
// Parameters:  HINSTANCE           Applicayion instance
//              HWND                Parent window handler
//              RECT*               a rect for a fixed size control, NULL will cause the control to ne 100% of
//                                  It's parent size
//              DWORD               Creation flags
//              TREELIST_CB*        a pointer to a call back to validate the user edit requests (can be NULL)
// Return:      TREELIST_HANDLE     a valid handle to the listtree control
//
////////////////////////////////////////////////////////////////////////////////////

CTreeListView::CTreeListView(HINSTANCE Instance, HWND Hwnd, RECT *pRect, DWORD dwFlags, TREELIST_CB *pFunc) : TreeListCRC32Table {
	0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,0xE963A535,
	0x9E6495A3,0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x09B64C2B,0x7EB17CBD,
	0xE7B82D07,0x90BF1D91,0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,
	0x6DDDE4EB,0xF4D4B551,0x83D385C7,0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,
	0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,0x3B6E20C8,0x4C69105E,0xD56041E4,
	0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,0x35B5A8FA,0x42B2986C,
	0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,0x26D930AC,
	0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,
	0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,
	0xB6662D3D,0x76DC4190,0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,
	0x9FBFE4A5,0xE8B8D433,0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,
	0x086D3D2D,0x91646C97,0xE6635C01,0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,
	0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,0x65B0D9C6,0x12B7E950,0x8BBEB8EA,
	0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,0x4DB26158,0x3AB551CE,
	0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,0x4369E96A,
	0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,
	0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,
	0xCE61E49F,0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,
	0xB7BD5C3B,0xC0BA6CAD,0xEDB88320,0x9ABFB3B6,0x03B6E20C,0x74B1D29A,0xEAD54739,
	0x9DD277AF,0x04DB2615,0x73DC1683,0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,
	0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,0xF00F9344,0x8708A3D2,0x1E01F268,
	0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,0xFED41B76,0x89D32BE0,
	0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,0xD6D6A3E8,
	0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,
	0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,
	0x4669BE79,0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,
	0x220216B9,0x5505262F,0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,
	0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,0x9B64C2B0,0xEC63F226,0x756AA39C,0x026D930A,
	0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,0x95BF4A82,0xE2B87A14,0x7BB12BAE,
	0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,0x86D3D2D4,0xF1D4E242,
	0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,0x88085AE6,
	0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,
	0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x4969474D,
	0x3E6E77DB,0xAED16A4A,0xD9D65ADC,0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,
	0x47B2CF7F,0x30B5FFE9,0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,
	0xCDD70693,0x54DE5729,0x23D967BF,0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,
	0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D} {

	
	bool                Error = false;
	bool                PrevInstance = false;

	if ((Hwnd == 0) || (Instance == 0))
		return ;



	InstanceParent = Instance;
	HwndParent = Hwnd;

	// This essentoal for the tree view
/*
	// Do we have a prev instance?
	if (TreeList_Internal_DictGetPtr(Hwnd, nullptr))
		PrevInstance = true;
*/
	if (PrevInstance == false) {
		InitCommonControls();
	}
	do {

		// Create a font for the TreeList control
		FontHandleTreeList = CreateFont(14, 0, 0, 0, 500, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_CHARACTER_PRECIS, PROOF_QUALITY, FF_DONTCARE, TEXT("Courier"));

		// Create a font for the Header control
		FontHandleHeader = CreateFont(16, 0, 0, 0, 700, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_CHARACTER_PRECIS, PROOF_QUALITY, FF_DONTCARE, TEXT("Arial"));

		// Create a font for the edit box
		FontHandleEdit = CreateFont(14, 0, 0, 0, 500, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_CHARACTER_PRECIS, PROOF_QUALITY, FF_DONTCARE, TEXT("Courier"));


		// Check that we got the fonts
		if ((FontHandleTreeList == nullptr) || (FontHandleHeader == nullptr) || (FontHandleEdit == nullptr)) {
			Error = true;
			break;
		}
		// Get the fonts info
		GetObject(FontHandleTreeList, sizeof(LOGFONT), &FontInfoTreeList);
		GetObject(FontHandleHeader, sizeof(LOGFONT), &FontInfoHeader);
		GetObject(FontHandleEdit, sizeof(LOGFONT), &FontInfoEdit);

		if (pRect) // User provided a control rect
			memcpy(&RectRequested, pRect, sizeof(RECT));

		else
			UseFullSize = true;


		// Create the list View and the header
		HwndTreeView = CreateWindowEx(0, WC_TREEVIEW, 0, WS_CHILD | WS_VISIBLE | TVS_FULLROWSELECT | TVS_NOHSCROLL | TVS_NOTOOLTIPS | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT, 0, 0, 0, 0, HwndParent, 0, InstanceParent, 0);

		if (!HwndTreeView) {
			Error = true;
			break;
		}


		//TreeList_Internal_DictUpdate(false, this, HwndParent, HwndTreeView);
		SendMessage(HwndTreeView, WM_SETFONT, reinterpret_cast<WPARAM>(FontHandleTreeList), static_cast<LPARAM>(true));
		ProcTreeList = reinterpret_cast<WNDPROC>(SetWindowLongPtr(HwndTreeView, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Static_TreeList_Internal_HandleTreeMessagesEx))); // Sub classing the control
		SetWindowLongPtr(HwndTreeView, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
																																				// Sub class the parent window
		if (PrevInstance == false) {
			ProcParent = reinterpret_cast<WNDPROC>(SetWindowLongPtr(HwndParent, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Static_TreeList_Internal_HandleTreeMessages))); // Sub classing the control
			SetProp(HwndParent, TEXT("WNDPROC"), ProcParent);
		}

		HwndHeader = CreateWindowEx(0, WC_HEADER, 0, WS_CHILD | WS_VISIBLE | HDS_FULLDRAG, 0, 0, 0, 0, HwndParent, 0, InstanceParent, 0);

		if (!HwndTreeView) {
			Error = true;
			break;
		}

		//TreeList_Internal_DictUpdate(false, this, HwndParent, HwndHeader);
		SendMessage(HwndHeader, WM_SETFONT, reinterpret_cast<WPARAM>(FontHandleHeader), static_cast<LPARAM>(true));

	} while (0);

	if (Error) {
		if (FontHandleTreeList)
			DeleteObject(FontHandleTreeList);
		if (FontHandleHeader)
			DeleteObject(FontHandleHeader);
		if (FontHandleEdit)
			DeleteObject(FontHandleEdit);
		return;
	}

	if (pFunc)
		pCBValidateEdit = pFunc;

	CreateFlags = dwFlags;
	if (((CreateFlags & TREELIST_ANCHOR_RIGHT) == TREELIST_ANCHOR_RIGHT) || ((CreateFlags & TREELIST_ANCHOR_BOTTOM) == TREELIST_ANCHOR_BOTTOM)) {
		UseAnchors = true;
	}


	// Edit box style
	if ((CreateFlags & TREELIST_NORMAL_EDITBOX) == TREELIST_NORMAL_EDITBOX)
		EditBoxStyleNormal = true;

	TreeView_SetBkColor(HwndTreeView, RGB(255, 255, 255));
	TreeView_SetTextColor(HwndTreeView, RGB(0, 0, 0));
	TreeList_Internal_RepositionControls();
}


////////////////////////////////////////////////////////////////////////////////////
//
// Function:    TreeListDestroy
// Description: Delete all elements and free memory
// Parameters:  TREELIST_HANDLE a valid handle to a listtree control
// Note:        void
//
//
////////////////////////////////////////////////////////////////////////////////////




CTreeListView::~CTreeListView() {
	// Kill windows objects
	if (FontHandleTreeList)
		DeleteObject(FontHandleTreeList);
	if (FontHandleHeader)
		DeleteObject(FontHandleHeader);
	if (FontHandleEdit)
		DeleteObject(FontHandleEdit);


	TreeList_Internal_DestroyEditBox(); // Kill the edit box

												// Kill the header window
	if (HwndHeader) {
		//TreeList_Internal_DictUpdate(true, this, NULL, HwndHeader);
		DestroyWindow(HwndHeader);

	}

	// Kill the TreeView main window
	if (HwndTreeView) {
		//TreeList_Internal_DictUpdate(true, this, nullptr, HwndTreeView);
		SetWindowLongPtr(HwndParent, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(ProcParent)); // Restore the original wnd proc to the parent
		RemoveProp(HwndParent, TEXT("WNDPROC"));
		DestroyWindow(HwndTreeView);
	}

	for (auto& item : ColumnsInfo) {
		if (item) {
			if (item) {
				delete item;
				item = nullptr;
				AllocatedTreeBytes -= sizeof(TreeListColumnInfo);
			}

		}
	}

	// Free all the nodes
	TreeList_Internal_NodeFreeAllSubNodes(pRootNode);
	InvalidateRect(HwndParent, &RectParent, true);


	assert(!AllocatedTreeBytes);

}





////////////////////////////////////////////////////////////////////////////////////
//
// Function:    TreeListAddColumn
// Description: Add a column to the TreeList control
// Parameters:  TREELIST_HANDLE a valid handle to a listtree control
//              char*           Null terminated string of the column name (up to TREELIST_MAX_STRING)
//              int             Width in pixels, last call to this function should set it to TREELIST_LAST_COLUMN
// Notes:       Max xolumns count is defined as TREELIST_MAX_COLUMNS
// Return:      TreeListError   see enum
//
////////////////////////////////////////////////////////////////////////////////////

TreeListError CTreeListView::AddColumn(const tstring& szColumnName, int Width) {



	if (ColumnsLocked == true)
		return e_ERROR_COULD_NOT_ADD_COLUMN;
/*
	if (ColumnsCount >= TREELIST_MAX_COLUMNS)
		return e_ERROR_COULD_NOT_ADD_COLUMN;
*/
	
	ColumnsInfo.push_back(new TreeListColumnInfo);
	if (!ColumnsInfo[ColumnsCount])
		return e_ERROR_MEMORY_ALLOCATION;

	AllocatedTreeBytes += sizeof(TreeListColumnInfo);
	memset(ColumnsInfo[ColumnsCount], 0, sizeof(TreeListColumnInfo));
	ColumnsInfo[ColumnsCount]->ColumnName = szColumnName;
	ColumnsInfo[ColumnsCount]->Width = Width;

	memset(&HeaderItem, 0, sizeof(HDITEM));
	HeaderItem.mask = HDI_WIDTH | HDI_FORMAT | HDI_TEXT;
	HeaderItem.cxy = Width;

	if (Width == TREELIST_LAST_COLUMN) {
		ColumnsLocked = true;
		HeaderItem.cxy = 100;
		ColumnsInfo[ColumnsCount]->Width = HeaderItem.cxy;

	}

	HeaderItem.fmt = HDF_CENTER;
	HeaderItem.pszText = const_cast<LPTSTR>(ColumnsInfo[ColumnsCount]->ColumnName.c_str());
	HeaderItem.cchTextMax = static_cast<int>(ColumnsInfo[ColumnsCount]->ColumnName.size());
	ColumnsTotalWidth += HeaderItem.cxy;
	Header_InsertItem(HwndHeader, ColumnsCount,reinterpret_cast<LPARAM>(&HeaderItem));

	ColumnsCount++;

	if (ColumnsLocked == true)
		TreeList_Internal_AutoSetLastColumn();

	return e_ERROR_COULD_NOT_ADD_COLUMN;
}



////////////////////////////////////////////////////////////////////////////////////
//
// Function:    TreeListAddNode
// Description: Add a new node
// Parameters:  TREELIST_HANDLE a valid handle to a listtree control
//              NODE_HANDLE         a Handle to the parent node, NULL when this is the first root node
//              TreeListNodeData*   an array of columns data attached to this node
//              int                 Count of elements in  TreeListNodeData*
// Return:      NODE_HANDLE         a valid node handle , NULL on error
//
////////////////////////////////////////////////////////////////////////////////////
TreeListNode* CTreeListView::AddNode(TreeListNode* pParentNode, const vector<TreeListNodeData*>& RowOfColumns) {

	TreeListNode*		pNewNode = nullptr;
	TVITEM              TreeItem;

	if (RowOfColumns.empty())// No data to add
		return nullptr;
	
	ColumnsLocked = true; // Lock columns

	pNewNode = TreeList_Internal_AddNode(pParentNode);
	if (pNewNode) {
		for (int i = 0; i < static_cast<int>(RowOfColumns.size()); ++i) {
			if (!TreeList_Internal_NodeColonize(pNewNode, RowOfColumns[i]))
				return nullptr; // Could not add the columns data
		}

		// Update UI Properties
		if (pNewNode->pNodeData[0]->type == IMAGELIST) {
			TreeItem.mask = TVIF_IMAGE | TVIF_PARAM;
			TreeItem.iImage = pNewNode->pNodeData[0]->pimagelist->GetCurrentImage();
		} else if (pNewNode->pNodeData[0]->type == TEXT) {
			TreeItem.mask = TVIF_TEXT | TVIF_PARAM;
			TreeItem.pszText = const_cast<LPTSTR>((pNewNode->pNodeData[0]->text.c_str()));
			TreeItem.cchTextMax = static_cast<int>(pNewNode->pNodeData[0]->text.size());
		} else if (pNewNode->pNodeData[0]->type == HWINDOW) {
			TreeItem.mask = TVIF_TEXT | TVIF_PARAM;
			TreeItem.pszText = 0;
			TreeItem.cchTextMax = 0;
		}
		TreeItem.lParam = reinterpret_cast<LPARAM>(pNewNode);

		// Updatge the base struct
		TreeStruct.item = TreeItem;
		TreeStruct.hInsertAfter = 0;
		if (!pParentNode)
			TreeStruct.hParent = TVI_ROOT;
		else
			TreeStruct.hParent = pParentNode->TreeItemHandle;


		pNewNode->TreeItemHandle = TreeView_InsertItem(HwndTreeView, &TreeStruct);
		if (!pNewNode->TreeItemHandle) // Error so ..
		{
			delete pNewNode;
			pNewNode = nullptr;
		}
	}

	return pNewNode;
}



/*
///////////////////////////////////////////////////////////////////////////////////////
//
// Function : TreeList_Internal_GetSessionFromDict
// Notes    : When hWndAny is NULL it will return a pointer to the dictionary
//          : When hWndParent is NULL it will be searched according to hWndAny
// Returns  : All the instances are being stored as an array of values under the parent hwnd
//
//
///////////////////////////////////////////////////////////////////////////////////////

static void *TreeList_Internal_DictGetPtr(HWND hWndParent, HWND hWndAny) {


HWND hParent = hWndParent;

TreeListDict *pTreeListDict = nullptr;

if (!hWndParent)
hParent = GetParent(hWndAny);

pTreeListDict = (TreeListDict*)GetProp(hParent, TREELIST_PROP_VAL); // Extract the dict pointer
if (!pTreeListDict)
return nullptr; // No pointr attached to the window handler or no instances

if (!hWndAny)
return reinterpret_cast<void*>(pTreeListDict);

if (pTreeListDict->ReferenceCount == 0)
return nullptr;

for (int iCount = 0; iCount < pTreeListDict->HwndInstances.size(); iCount++) {

for (int iElement = 0; iElement < pTreeListDict->HwndInstances[iCount].size();iElement++) {
if (pTreeListDict->HwndInstances[iCount][iElement] == hWndAny) {
if (pTreeListDict->SessionPtr[iCount])
return reinterpret_cast<void*>(pTreeListDict->SessionPtr[iCount]);

}
}

}

return nullptr;
}


///////////////////////////////////////////////////////////////////////////////////////
//
// Function : TreeList_Internal_DictUpdate
// Notes    : When hWndParent is NULL it will be searched according to hWndAny
// Notes  :   Update the dictionary pointer to hold or remove a new session
//
//
///////////////////////////////////////////////////////////////////////////////////////

bool TreeList_Internal_DictUpdate(bool Clear, CTreeListView* pSession, HWND hWndParent, HWND hWndAny) {

HWND hParent = hWndParent;
int iNullsCount = 0;
TreeListDict *pTreeListDict = 0;


if (!hWndAny || !pSession)
return false;

if (!hWndParent)
hParent = GetParent(hWndAny); // Search for the parent


pTreeListDict = (TreeListDict*)GetProp(hParent, TREELIST_PROP_VAL); // Extract the dict pointer from the parent
if (Clear == false) {
if (!pTreeListDict) {
// Allocate and attach
pTreeListDict = new TreeListDict;
std::memset(pTreeListDict, 0, sizeof(TreeListDict));
if (SetProp(hParent, TREELIST_PROP_VAL, pTreeListDict) == false)
return false;

pTreeListDict->SessionPtr.push_back(pSession);
pTreeListDict->HwndInstances.push_back({hWndAny});
pTreeListDict->HwndParent.push_back(hParent);
pTreeListDict->ReferenceCount++;

return true;
}

for (int iCount = 0;iCount < pTreeListDict->SessionPtr.size();iCount++) {

// Look for the session pointer
if (pTreeListDict->SessionPtr[iCount] == pSession) {
// found it, look if it holds our hwnd
for (int iElement = 0;iElement < pTreeListDict->HwndInstances[iCount].size();iElement++) {
if (pTreeListDict->HwndInstances[iCount][iElement] == hWndAny)
return false; // All ready there
}

// found it, update the array
pTreeListDict->HwndInstances[iCount].push_back(hWndAny);
return true; // Updated
}
}


// The session was not found, we have to add a new one

// Look for an empty place
pTreeListDict->SessionPtr.push_back(pSession);
pTreeListDict->HwndInstances.push_back({ hWndAny });
pTreeListDict->HwndParent.push_back(hParent);
pTreeListDict->ReferenceCount++;
return true;

} else { // Clear the param

for (int iCount = 0;iCount<pTreeListDict->SessionPtr.size();iCount++) {

// Look for the session pointer
if (pTreeListDict->SessionPtr[iCount] == pSession) {
// found it, look if it holds our hwnd
for (int iElement = 0;iElement < pTreeListDict->HwndInstances[iCount].size();iElement++) {

if (pTreeListDict->HwndInstances[iCount][iElement] == hWndAny) {
pTreeListDict->HwndInstances[iCount].erase(pTreeListDict->HwndInstances[iCount].begin() + iElement);
break;
}
}


if (pTreeListDict->HwndInstances[iCount].empty()) {

// Clean it, reduce refcount
pTreeListDict->SessionPtr[iCount] = nullptr;
pTreeListDict->HwndParent[0] = 0;
pTreeListDict->ReferenceCount--;
if (pTreeListDict->ReferenceCount == 0) {
delete pTreeListDict;
RemoveProp(hParent, TREELIST_PROP_VAL);
}
}

return true;
}
}

return false;
}
}

*/