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
#define TREELIST_ELEMENTS_PER_INSTANCE          4
#define TREELIST_PROP_VAL                       TEXT("TREELIST_PTR")

//#define TREELIST_DOUBLE_BUFFEING

typedef HANDLE(CALLBACK* LPREMOVEPROP)(HWND, LPCTSTR);  // VC2010 issue





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
						pRemoveProp(hParent, TREELIST_PROP_VAL);
						if (hDllHandle)
							FreeLibrary(hDllHandle);
					}
				}

				return true;
			}
		}

		return false;
	}
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

		TreeList_Internal_DictUpdate(true, this, nullptr, HwndEditBox);
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
		delete pNode->pNodeData[Node];
		pNode->pNodeData[Node] = nullptr;
		AllocatedTreeBytes -= sizeof(TreeListNodeData);

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
		dwStyle = GetWindowLongPtr(HwndTreeView, GWL_STYLE);
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
// Function: TreeList_Internal_HandleEditBoxMessages
// Description:Subclassing the editbox to the get the key press notifications
// Parameters:
// Return:
// Logic:
//
////////////////////////////////////////////////////////////////////////////////////

static LRESULT CALLBACK TreeList_Internal_HandleEditBoxMessages(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {


	TreeListSession     *pSession = 0;

	pSession = (TreeListSession*)TreeList_Internal_DictGetPtr(NULL, hWnd);

	if (!pSession)
		return FALSE;

	switch (Msg) {
	case WM_KEYUP: // Trap the enter key (this is needed when we are working with a dialog)
	{
		if (wParam == 0x0d) // Enter key was pressed
			PostMessage(pSession->HwndParent, TREELIST_WM_EDIT_ENTER, 0, (LPARAM)pSession);
	}
	break;
	}

	return CallWindowProc(pSession->ProcEdit, hWnd, Msg, wParam, lParam);
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

static LRESULT CALLBACK TreeList_Internal_HandleTreeMessagesEx(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {


	TreeListSession     *pSession = 0;

	pSession = (TreeListSession*)TreeList_Internal_DictGetPtr(NULL, hWnd);
	if (!pSession)
		return FALSE;


	if (!pSession)
		return FALSE;

	switch (Msg) {

	case WM_VSCROLL: // Trap the vertical scroll
	{

		TreeList_Internal_DestroyEditBox(pSession); // Kill the edit box when the user scrolls vertically
	}
	break;

	case WM_LBUTTONUP:
	{
		pSession->ItemWasSelected = TRUE;

	}
	break;
	}


	return CallWindowProc(pSession->ProcTreeList, hWnd, Msg, wParam, lParam);
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

static LRESULT TreeList_Internal_HandleTreeMessages(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {


	TreeListSession     *pSession = 0;
	TreeListDict        *pDict = 0;
	LPNMTVCUSTOMDRAW    lpNMTVCustomDraw = 0;
	HTREEITEM           hTreeItem = 0;
	HDITEM              HeaderItem;
	LPNMHDR             lpNMHeader = 0;
	LPNMHEADER          lpNMHeaderChange = 0;
	TVHITTESTINFO       TreeTestInfo = { 0 };
	TVITEM              Treeitem = { 0 };
	WNDPROC             ProcParent = (WNDPROC)GetProp(hWnd, "WNDPROC");
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
	BOOL                RetVal = FALSE;
	BOOL                SelectedLine = FALSE;
	int                 iCol = 0;
	int                 iOffSet = 0;
	int                 iColEnd, iColStart = 0;
	int                 iControlID = 0;
	TreeListSession     *pFirstSession = 0;
	TreeListNode        *pNode = 0;
	LPWINDOWPOS         lpPos = 0;


	pDict = (TreeListDict*)TreeList_Internal_DictGetPtr(hWnd, NULL);
	if (!pDict)
		return FALSE;

	switch (Msg) {



		///////////////////////////////////////////

	case WM_SIZE:
	{
		// Maximize
		if (wParam == SIZE_MAXIMIZED) {
			PostMessage(hWnd, WM_EXITSIZEMOVE, 0, 0);
			break;
		}
		// Restore down
		for (iControlID = 0; iControlID < TREELIST_MAX_INSTANCES;iControlID++) {
			pSession = pDict->pSessionPtr[iControlID];
			if (!pSession || pSession->ParentResizing == TRUE)
				break;

			PostMessage(hWnd, WM_EXITSIZEMOVE, 0, 0);
			break;

		}

	}
	break;
	///////////////////////////////////////////


	case WM_ENTERSIZEMOVE:
	{
		for (iControlID = 0; iControlID < TREELIST_MAX_INSTANCES;iControlID++) {
			pSession = pDict->pSessionPtr[iControlID];
			if (pSession)
				pSession->ParentResizing = TRUE;

		}
		break;

	}
	break;
	///////////////////////////////////////////

	case WM_EXITSIZEMOVE:
	{


		for (iControlID = 0; iControlID < TREELIST_MAX_INSTANCES;iControlID++) {
			pSession = pDict->pSessionPtr[iControlID];

			if (!pSession)
				break;

			pSession->ParentResizing = FALSE;

			if (!pFirstSession)
				pFirstSession = pSession;

			// Check if we have to reposition the controls
			GetClientRect(pSession->HwndParent, &RectParent);
			if (memcmp(&RectParent, &pSession->RectParent, sizeof(RECT)) != 0) {

				if ((pSession->UseFullSize == TRUE) || (pSession->UseAnchors == TRUE)) {
					pSession->ItemWasSelected = FALSE;
					TreeList_Internal_RepositionControls(pSession);
				}
			}
		}

		if (pFirstSession)
			InvalidateRect(pFirstSession->HwndParent, &pFirstSession->RectParent, TRUE);
	}
	break;


	///////////////////////////////////////////

	case WM_PAINT:
	{


		for (iControlID = 0; iControlID < TREELIST_MAX_INSTANCES;iControlID++) {
			pSession = pDict->pSessionPtr[iControlID];
			if (!pSession || pSession->ParentResizing)
				break;

			if (pSession->ColumnDoAutoAdjust == TRUE)
				TreeList_Internal_AutoSetLastColumn(pSession);
		}

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


		for (iControlID = 0; iControlID < TREELIST_MAX_INSTANCES;iControlID++) {
			pSession = pDict->pSessionPtr[iControlID];
			if (!pSession || pSession->ParentResizing)
				break;
			if (((HWND)lParam == pSession->HwndEditBox) && (pSession->EditBoxStyleNormal == FALSE)) {
				RetVal = TRUE;
				SetTextColor((HDC)wParam, RGB(255, 255, 255));
				SetBkColor((HDC)wParam, RGB(0, 0, 0));
			}
		}
	}
	break;

	///////////////////////////////////////////

	case TREELIST_WM_EDIT_NODE:
	{

		pSession = (TreeListSession*)lParam;

		if (!pSession || pSession->ParentResizing)
			break;

		if (pSession->HwndEditBox)
			break;


		RetVal = TRUE;
		pNode = TreeList_Internal_GetNodeFromTreeHandle(pSession, pSession->EditedTreeItem);
		if (pNode) {
			// This is place to edit a cell that needs editing
			if (Header_GetItemRect(pSession->HwndHeader, pSession->EditedColumn, &RectHeaderItem) == FALSE) {
				pSession->EditedColumn = 0;
				pSession->EditedTreeItem = 0;
				break;
			}
			if (TreeView_GetItemRect(pSession->HwndTreeView, pSession->EditedTreeItem, &RectText, TRUE) == FALSE) {
				pSession->EditedColumn = 0;
				pSession->EditedTreeItem = 0;
				break;
			}
			pSession->SizeEdit.Width = RectHeaderItem.right - RectHeaderItem.left;
			pSession->SizeEdit.Hight = RectText.bottom - RectText.top;
			pSession->SizeEdit.X = RectHeaderItem.left;
			pSession->SizeEdit.Y = RectText.top;

			// Check if we have vertical scrollbars
			// Only if we are editing the last caolumn (4/26/20110)
			iOffSet = 0;
			if (pSession->EditedColumn == (pSession->ColumnsCount - 1)) {
				dwStyle = GetWindowLong(pSession->HwndTreeView, GWL_STYLE);
				if ((dwStyle & WS_VSCROLL) != 0)
					iOffSet = GetSystemMetrics(SM_CXVSCROLL);
			}
			// a Numeric only text box
			if (pNode->pNodeData[pSession->EditedColumn]->Numeric == TRUE)
				dwStyle = WS_CHILD | WS_VISIBLE | ES_NUMBER;
			else
				dwStyle = WS_CHILD | WS_VISIBLE;

			pSession->HwndEditBox = CreateWindowEx(0,
				"EDIT",
				pNode->pNodeData[pSession->EditedColumn]->Data,
				dwStyle,
				pSession->SizeEdit.X + pSession->SizeRequested.X,                                                       // Position: left
				pSession->SizeEdit.Y + ((RectHeaderItem.bottom - RectHeaderItem.top)) + pSession->SizeRequested.Y,    // Position: top
				pSession->SizeEdit.Width - 1 - iOffSet,    // Width
				pSession->SizeEdit.Hight - 2,            // Height
				pSession->HwndParent,                  // Parent window handle
				0,
				pSession->InstanceParent, 0);

			if (pSession->HwndEditBox) {

				TreeList_Internal_DictUpdate(FALSE, pSession, pSession->HwndParent, pSession->HwndEditBox);
				pSession->ProcEdit = (WNDPROC)SetWindowLongPtr(pSession->HwndEditBox, GWLP_WNDPROC, (LONG)TreeList_Internal_HandleEditBoxMessages); // Subclassing the control (it will help trapping the enter key press event on a dialog)
				SendMessage(pSession->HwndEditBox, WM_SETFONT, (WPARAM)pSession->FontHandleEdit, (LPARAM)TRUE);
				SetFocus(pSession->HwndEditBox);
				SendMessage(pSession->HwndEditBox, EM_SETSEL, strlen(pNode->pNodeData[pSession->EditedColumn]->Data), strlen(pNode->pNodeData[pSession->EditedColumn]->Data)); // Select all text

			} else {
				pSession->EditedColumn = 0;
				pSession->EditedTreeItem = 0;
			}

		}

	}
	break;
	///////////////////////////////////////////
	case TREELIST_WM_EDIT_ENTER:
	{

		pSession = (TreeListSession*)lParam;
		if (!pSession || pSession->ParentResizing)
			break;

		if (pSession->WaitingForCaller == TRUE)// Do nothong while waiting for the caller
			break;

		pNode = TreeList_Internal_GetNodeFromTreeHandle(pSession, pSession->EditedTreeItem);
		if (!pNode)
			break;

		RetVal = TRUE;
		memset(pSession->EditBoxBuffer, 0, sizeof(pSession->EditBoxBuffer));
		if (GetWindowText(pSession->HwndEditBox, pSession->EditBoxBuffer, TREELIST_MAX_STRING) > 0)// Get edit box text
		{
			if (strcmp(pSession->EditBoxBuffer, pNode->pNodeData[pSession->EditedColumn]->Data) != 0) {
				// Call the user!
				if (pSession->pCBValidateEdit) {
					pSession->WaitingForCaller = TRUE;
					pSession->EditBoxOverrideBuffer[0] = 0;

					if (pSession->pCBValidateEdit((NODE_HANDLE)pSession, pNode->pNodeData[pSession->EditedColumn]->pExternalPtr, pSession->EditBoxBuffer, pSession->EditBoxOverrideBuffer) == TRUE) {

						if (pSession->EditBoxOverrideBuffer[0])
							strncpy(pNode->pNodeData[pSession->EditedColumn]->Data, pSession->EditBoxOverrideBuffer, TREELIST_MAX_STRING);
						else
							strncpy(pNode->pNodeData[pSession->EditedColumn]->Data, pSession->EditBoxBuffer, TREELIST_MAX_STRING);
						pNode->pNodeData[pSession->EditedColumn]->Altered = TRUE; // Set "changed" flag so we will be able to color it later
																				  // Update CRC
						pNode->pNodeData[pSession->EditedColumn]->CRC = TreeList_Internal_CRCCreate(pNode->pNodeData[pSession->EditedColumn],
							(sizeof(TreeListNodeData) - sizeof(pNode->pNodeData[pSession->EditedColumn]->CRC))); // Set the srs sig
					}
					pSession->WaitingForCaller = FALSE;
				}
			}
		}


		TreeList_Internal_DestroyEditBox(pSession);// Kill the edit box

												   // Select the edited line in the Tree
		if (pNode && pSession->EditedColumn > 0)
			TreeView_SelectItem(pSession->HwndTreeView, pNode->TreeItemHandle);

		SetFocus(pSession->HwndTreeView);
		pSession->EditedColumn = 0;
		pSession->EditedTreeItem = 0;
		break;

	}
	break;
	///////////////////////////////////////////
	case WM_NOTIFY:
	{


		lpNMHeader = (LPNMHDR)lParam;
		pSession = (TreeListSession*)TreeList_Internal_DictGetPtr(hWnd, lpNMHeader->hwndFrom);
		if (!pSession || pSession->ParentResizing)
			break;


		switch (lpNMHeader->code) {


			///////////////////////////////////////////
		case NM_CUSTOMDRAW:
		{

			RetVal = TRUE;
			lpNMTVCustomDraw = (LPNMTVCUSTOMDRAW)lParam;

			if (lpNMHeader->hwndFrom != pSession->HwndTreeView)
				break;

			switch (lpNMTVCustomDraw->nmcd.dwDrawStage) {

			case CDDS_PREPAINT:
				return CDRF_NOTIFYITEMDRAW;

			case CDDS_ITEMPREPAINT:
				return (CDRF_DODEFAULT | CDRF_NOTIFYPOSTPAINT);

			case CDDS_ITEMPOSTPAINT:
			{


				// Get a valid pinter to our internal data type
				hTreeItem = (HTREEITEM)lpNMTVCustomDraw->nmcd.dwItemSpec;
				pNode = TreeList_Internal_GetNodeFromTreeHandle(pSession, hTreeItem);
				if (!pNode || pNode->NodeDataCount == 0)
					return(CDRF_DODEFAULT);

				memcpy(&RectItem, &lpNMTVCustomDraw->nmcd.rc, sizeof(RECT));
				if (IsRectEmpty(&RectItem) == TRUE) {
					// Nothing to paint when we have an empty rect
					SetWindowLongPtr(pSession->HwndParent, DWLP_MSGRESULT, CDRF_DODEFAULT);
					break;
				}

				hDC = lpNMTVCustomDraw->nmcd.hdc;
				if (!hDC)
					return(CDRF_DODEFAULT); // No HDC

				SetBkMode(hDC, TRANSPARENT);

				if (TreeView_GetItemRect(pSession->HwndTreeView, hTreeItem, &RectLabel, TRUE) == FALSE)
					return(CDRF_DODEFAULT); // No RECT


				SetTextColor(hDC, RGB(0, 0, 0)); // Make sure we use black color
				clTextBk = lpNMTVCustomDraw->clrTextBk;
				clWnd = TreeView_GetBkColor(pSession->HwndTreeView);
				brTextBk = CreateSolidBrush(clTextBk);
				brWnd = CreateSolidBrush(clWnd);

				// Clear the original label rectangle
				RectLabel.right = pSession->RectTree.right;
				FillRect(hDC, &RectLabel, brWnd);

				pSession->ColumnsCount = Header_GetItemCount(pSession->HwndHeader);
				if (pSession->ColumnsCount == -1)
					return(CDRF_DODEFAULT); // No columns info, nothing to do


											// Draw the horizontal lines
				for (iCol = 0; iCol < pSession->ColumnsCount; iCol++) {
					// Get current columns width from the header window
					memset(&HeaderItem, 0, sizeof(HeaderItem));
					HeaderItem.mask = HDI_HEIGHT | HDI_WIDTH;
					if (Header_GetItem(pSession->HwndHeader, iCol, &HeaderItem) == TRUE) {

						pSession->pColumnsInfo[iCol]->Width = HeaderItem.cxy;
						iOffSet += HeaderItem.cxy;
						RectItem.right = iOffSet - 1;
						DrawEdge(hDC, &RectItem, BDR_SUNKENINNER, BF_RIGHT);
					}
				}

				// Draw the vertical lines
				DrawEdge(hDC, &RectItem, BDR_SUNKENINNER, BF_BOTTOM);

				// Draw Label, calculate the rect first
				DrawText(hDC, pNode->pNodeData[0]->Data, strlen(pNode->pNodeData[0]->Data), &RectText, DT_NOPREFIX | DT_CALCRECT);
				RectLabel.right = TREELIST_MIN((RectLabel.left + RectText.right + 4), pSession->pColumnsInfo[0]->Width - 4);

				if ((RectLabel.right - RectLabel.left) < 0)
					brTextBk = brWnd;

				if (clTextBk != clWnd)  // Draw label's background
				{

					if (pSession->ItemWasSelected == TRUE) {
						SelectedLine = TRUE;
						SetTextColor(hDC, RGB(255, 255, 255));
						RectLabel.right = pSession->RectTree.right;
						FillRect(hDC, &RectLabel, brTextBk);
					}
				}

				// Draw main label
				memcpy(&RectText, &RectLabel, sizeof(RECT));

				// The label right shoud be as the column right
				if (Header_GetItemRect(pSession->HwndHeader, 0, &RectHeaderItem) == FALSE)
					return(CDRF_DODEFAULT);// Error getting the rect


				RectText.right = RectHeaderItem.right; // Set the right side
				TreeList_Internal_DeflateRectEx(&RectText, 2, 1); // Defalate it
				DrawText(hDC, pNode->pNodeData[0]->Data, strlen(pNode->pNodeData[0]->Data), &RectText, DT_NOPREFIX | DT_END_ELLIPSIS); // Draw it
				iOffSet = pSession->pColumnsInfo[0]->Width;

				// Draw thwe other labels (the columns)
				for (iCol = 1; iCol < pSession->ColumnsCount; iCol++) {

					if (pNode->pNodeData[iCol]) {
						memcpy(&RectText, &RectLabel, sizeof(RECT));
						RectText.left = iOffSet;
						RectText.right = iOffSet + pSession->pColumnsInfo[iCol]->Width;

						// Set cell bk color
						if ((SelectedLine == FALSE) && (pNode->pNodeData[iCol]->Colored == TRUE)) {
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
							if (SelectedLine == FALSE) {
								if (pNode->pNodeData[iCol]->Colored == TRUE)
									SetTextColor(hDC, pNode->pNodeData[iCol]->TextColor);

								// Set special color for altered cells (if set by ty the caller)
								if (pNode->pNodeData[iCol]->Altered == TRUE)
									SetTextColor(hDC, pNode->pNodeData[iCol]->AltertedTextColor);

							}

							DrawText(hDC, pNode->pNodeData[iCol]->Data, strlen(pNode->pNodeData[iCol]->Data), &RectText, DT_NOPREFIX | DT_END_ELLIPSIS);
						}
						iOffSet += pSession->pColumnsInfo[iCol]->Width;
					}
				}

				SetTextColor(hDC, RGB(0, 0, 0));

				// Draw the rect (on the parent) around the tree
				if ((pSession->CreateFlags & TREELIST_DRAW_EDGE) == TREELIST_DRAW_EDGE)
					DrawEdge(GetDC(pSession->HwndParent), &pSession->RectBorder, EDGE_ETCHED, BF_RECT);

				return(CDRF_DODEFAULT);
			}
			break;
			}
		}
		break;

		///////////////////////////////////////////
		case NM_RELEASEDCAPTURE:
		{


			RetVal = TRUE;
			TreeList_Internal_DestroyEditBox(pSession); // Kill the edit box, when resizing a column

			TreeList_Internal_UpdateColumns(pSession);
			pSession->ColumnDoAutoAdjust = TRUE;

			// Refresh
			InvalidateRect(pSession->HwndParent, &pSession->RectClientOnParent, TRUE);



		}
		break;

		///////////////////////////////////////////
		case NM_CLICK:
		{

			if (lpNMHeader->hwndFrom != pSession->HwndTreeView)
				break;

			if (pSession->WaitingForCaller == TRUE)// Do nothong while waiting for the caller
				break;

			TreeList_Internal_DestroyEditBox(pSession);// Kill the edit box

			RetVal = FALSE;  // Will make our control respond to click same as dblcick
			dwMousePos = GetMessagePos();
			TreeTestInfo.pt.x = GET_X_LPARAM(dwMousePos);
			TreeTestInfo.pt.y = GET_Y_LPARAM(dwMousePos);
			MapWindowPoints(HWND_DESKTOP, lpNMHeader->hwndFrom, &TreeTestInfo.pt, 1);
			TreeView_HitTest(lpNMHeader->hwndFrom, &TreeTestInfo);

			if (TreeTestInfo.hItem) {
				pNode = TreeList_Internal_GetNodeFromTreeHandle(pSession, TreeTestInfo.hItem);
				if (!pNode)
					break;

				pSession->ItemWasSelected = TRUE;
				InvalidateRect(pSession->HwndTreeView, &pSession->RectTree, FALSE);

				// Get the correct column where the mouse has clicked..
				iColStart = pSession->RectHeader.left;
				iColEnd = 0;
				for (iCol = 0;iCol < pSession->ColumnsCount; iCol++) {
					iColEnd += pSession->pColumnsInfo[iCol]->Width;

					if ((TreeTestInfo.pt.x >= iColStart) && (TreeTestInfo.pt.x < iColEnd)) {
						if (pNode->NodeDataCount >= iCol) // Is there any data there?
						{
							if (pNode->pNodeData[iCol] && pNode->pNodeData[iCol]->Editable == TRUE) // Is it editable?
							{

								// Send edit message to the parent window
								pSession->EditedColumn = iCol;
								pSession->EditedTreeItem = TreeTestInfo.hItem;
								PostMessage(pSession->HwndParent, TREELIST_WM_EDIT_NODE, 0, (LPARAM)pSession);
								break;
							}
						}
					}
					iColStart = iColEnd;
				}

				if (pSession->HwndEditBox)
					SendMessage(pSession->HwndParent, TREELIST_WM_EDIT_ENTER, 0, 0);
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

	if (RetVal == FALSE) {
		if (ProcParent)
			return CallWindowProc(ProcParent, hWnd, Msg, wParam, lParam);
		else
			return FALSE;
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

CTreeListView::CTreeListView(HINSTANCE Instance, HWND Hwnd, RECT *pRect, DWORD dwFlags, TREELIST_CB *pFunc) {

	
	BOOL                Error = false;
	BOOL                PrevInstance = false;

	if ((Hwnd == 0) || (Instance == 0))
		return ;



	InstanceParent = Instance;
	HwndParent = Hwnd;

	// This essentoal for the tree view

	// Do we have a prev instance?
	if (TreeList_Internal_DictGetPtr(Hwnd, nullptr))
		PrevInstance = true;

	if (PrevInstance == false)
		InitCommonControls();
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


		TreeList_Internal_DictUpdate(false, this, HwndParent, HwndTreeView);
		SendMessage(HwndTreeView, WM_SETFONT, reinterpret_cast<WPARAM>(FontHandleTreeList), static_cast<LPARAM>(true));
		ProcTreeList = reinterpret_cast<WNDPROC>(SetWindowLongPtr(HwndTreeView, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(TreeList_Internal_HandleTreeMessagesEx))); // Sub classing the control

																																				// Sub class the parent window
		if (PrevInstance == false) {
			ProcParent = reinterpret_cast<WNDPROC>(SetWindowLongPtr(HwndParent, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(TreeList_Internal_HandleTreeMessages))); // Sub classing the control
			SetProp(HwndParent, TEXT("WNDPROC"), ProcParent);
		}

		HwndHeader = CreateWindowEx(0, WC_HEADER, 0, WS_CHILD | WS_VISIBLE | HDS_FULLDRAG, 0, 0, 0, 0, HwndParent, 0, InstanceParent, 0);

		if (!HwndTreeView) {
			Error = true;
			break;
		}

		TreeList_Internal_DictUpdate(false, this, HwndParent, HwndHeader);
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
	if ((CreateFlags & TREELIST_ANCHOR_RIGHT) == TREELIST_ANCHOR_RIGHT) || ((CreateFlags & TREELIST_ANCHOR_BOTTOM) == TREELIST_ANCHOR_BOTTOM)) {
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
	int AllocatedBytes;

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
		TreeList_Internal_DictUpdate(true, this, NULL, HwndHeader);
		DestroyWindow(HwndHeader);

	}

	// Kill the TreeView main window
	if (HwndTreeView) {
		TreeList_Internal_DictUpdate(true, this, nullptr, HwndTreeView);
		SetWindowLongPtr(HwndParent, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(ProcParent)); // Restore the original wnd proc to the parent
		pRemoveProp(HwndParent, TEXT("WNDPROC"));
		DestroyWindow(HwndTreeView);
	}



	// Free all the nodes
	TreeList_Internal_NodeFreeAllSubNodes(pRootNode);
	AllocatedBytes = AllocatedTreeBytes;
	InvalidateRect(HwndParent, &RectParent, true);


	assert(!AllocatedBytes);

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



	if (ColumnsLocked == TRUE)
		return e_ERROR_COULD_NOT_ADD_COLUMN;

	if (ColumnsCount >= TREELIST_MAX_COLUMNS)
		return e_ERROR_COULD_NOT_ADD_COLUMN;

	
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
		ColumnsLocked = TRUE;
		HeaderItem.cxy = 100;
		ColumnsInfo[ColumnsCount]->Width = HeaderItem.cxy;

	}

	HeaderItem.fmt = HDF_CENTER;
	HeaderItem.pszText = const_cast<LPTSTR>(ColumnsInfo[ColumnsCount]->ColumnName.c_str());
	HeaderItem.cchTextMax = static_cast<int>(ColumnsInfo[ColumnsCount]->ColumnName.size());
	ColumnsTotalWidth += HeaderItem.cxy;
	Header_InsertItem(HwndHeader, ColumnsCount,reinterpret_cast<LPARAM>(&HeaderItem));

	ColumnsCount++;

	if (ColumnsLocked == TRUE)
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
TreeListNode* CTreeListView::AddNode(TreeListNode* pParentNode, TreeListNodeData *RowOfColumns, int ColumnsCount) {

	TreeListNode*		pNewNode = nullptr;
	TVITEM              TreeItem;
	int                 Node;

	if (!RowOfColumns || ColumnsCount == 0)// No data to add
		return nullptr;

	ColumnsLocked = true; // Lock columns

	pNewNode = TreeList_Internal_AddNode(pParentNode);
	if (pNewNode) {
		for (Node = 0;Node <= ColumnsCount;Node++) {
			if (!TreeList_Internal_NodeColonize(pNewNode, RowOfColumns + Node))
				return nullptr; // Could not add the columns data
		}

		// Update UI Properties
		TreeItem.mask = TVIF_TEXT | TVIF_PARAM;
		TreeItem.pszText = const_cast<LPTSTR>((pNewNode->pNodeData[0]->Data).c_str());
		TreeItem.cchTextMax = static_cast<int>(pNewNode->pNodeData[0]->Data.size());
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

