#ifndef _MEMDC_H_
#define _MEMDC_H_

//////////////////////////////////////////////////
// CMyMemDC - memory DC
//
// Author: Keith Rule
// Email:  keithr@europa.com
// Copyright 1996-1999, Keith Rule
//
// You may freely use or modify this code provided this
// Copyright is included in all derived versions.
//
// History - 10/3/97 Fixed scrolling bug.
//                   Added print support. - KR
//
//           11/3/99 Fixed most common complaint. Added
//                   background color fill. - KR
//
//           11/3/99 Added support for mapping modes other than
//                   MM_TEXT as suggested by Lee Sang Hun. - KR
//
// This class implements a memory Device Context which allows
// flicker free drawing.

class CMyMemDC : public CDC {
private:	
	CBitmap		m_bitmap;		// Offscreen bitmap
	CBitmap*	m_oldBitmap;	// bitmap originally found in CMyMemDC
	CDC*		m_pDC;			// Saves CDC passed in constructor
	CRect		m_rect;			// Rectangle of drawing area.
	BOOL		m_bMemDC;		// TRUE if CDC really is a Memory DC.
public:
	
	CMyMemDC(CDC* pDC, const CRect* pRect = NULL, BOOL transparent = FALSE) : CDC()
	{
		ASSERT(pDC != NULL); 

		// Some initialization
		m_pDC = pDC;
		m_oldBitmap = NULL;
		m_bMemDC = !pDC->IsPrinting();

		// Get the rectangle to draw
		if (pRect == NULL) {
			pDC->GetClipBox(&m_rect);
		} else {
			m_rect = *pRect;
		}
		
		if (m_bMemDC) {
			// Create a Memory DC
			CreateCompatibleDC(pDC);
			pDC->LPtoDP(&m_rect);

			m_bitmap.CreateCompatibleBitmap(pDC, m_rect.Width(), m_rect.Height());
			m_oldBitmap = SelectObject(&m_bitmap);
			
			SetMapMode(pDC->GetMapMode());
			pDC->DPtoLP(&m_rect);
			SetWindowOrg(m_rect.left, m_rect.top);
		} else {
			// Make a copy of the relevent parts of the current DC for printing
			m_bPrinting = pDC->m_bPrinting;
			m_hDC       = pDC->m_hDC;
			m_hAttribDC = pDC->m_hAttribDC;
		}
		
		SetBkMode(TRANSPARENT);

		// Fill background 
		if (transparent){	
			BitBlt(m_rect.left, m_rect.top, m_rect.Width(), m_rect.Height(),
				m_pDC, 0, 0, SRCCOPY);			
		}
		else{
			//SetBkMode(TRANSPARENT);
			FillSolidRect(m_rect, pDC->GetBkColor()); // Not needed in OPAQUE mode???  See documentation
		}
	}

	
	~CMyMemDC()	
	{		
		if (m_bMemDC) {
			// Copy the offscreen bitmap onto the screen.
			m_pDC->BitBlt(m_rect.left, m_rect.top, m_rect.Width(), m_rect.Height(),
				this, m_rect.left, m_rect.top, SRCCOPY);			
			
			//Swap back the original bitmap.
			SelectObject(m_oldBitmap);
		} else {
			// All we need to do is replace the DC with an illegal value,
			// this keeps us from accidently deleting the handles associated with
			// the CDC that was passed to the constructor.			
			m_hDC = m_hAttribDC = NULL;
		}	
	}
	
	// Allow usage as a pointer	
	CMyMemDC* operator->() 
	{
		return this;
	}	

	// Allow usage as a pointer	
	operator CMyMemDC*() 
	{
		return this;
	}
};

class CMyBmpDC: public CDC {
private:	
	HBITMAP		m_hBmp;
	HGDIOBJ		m_hOldBmp;
		
	CDC			*m_pDC;
	CRect		m_rc;

public:

	CMyBmpDC() {
		m_hBmp = NULL;
		m_hOldBmp = NULL;
	}
	CMyBmpDC(CDC *pDC, UINT uIdResource) : CDC() {
		Create(pDC, uIdResource);
	};
	CMyBmpDC(CDC *pDC, TCHAR *strFileName) : CDC() {
		Create(pDC, strFileName);
	};

	~CMyBmpDC() {
		SelectObject(m_hOldBmp);

		::DeleteObject(m_hBmp);
	};

	void Create(CDC *pDC, UINT uIdResource) {
		BITMAP bmp;

		pDC = m_pDC;

		CreateCompatibleDC(pDC);

		m_hBmp = LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(uIdResource));
		m_hOldBmp = SelectObject(m_hBmp);

		GetObject(m_hBmp, sizeof(BITMAP), &bmp);

		m_rc.left = 0;
		m_rc.top = 0;
		m_rc.right = bmp.bmWidth;
		m_rc.bottom = bmp.bmHeight;
	};

	void Create(CDC *pDC, TCHAR *strFileName) {
		BITMAP bmp;

		pDC = m_pDC;

		CreateCompatibleDC(pDC);

		m_hBmp = (HBITMAP)LoadImage(AfxGetInstanceHandle(), strFileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		m_hOldBmp = SelectObject(m_hBmp);

		GetObject(m_hBmp, sizeof(BITMAP), &bmp);

		m_rc.left = 0;
		m_rc.top = 0;
		m_rc.right = bmp.bmWidth;
		m_rc.bottom = bmp.bmHeight;
	};

	// Allow usage as a pointer	
	CMyBmpDC* operator->() 
	{
		return this;
	}	

	// Allow usage as a pointer	
	operator CMyBmpDC*() 
	{
		return this;
	}

	int GetWidth() {
		return m_rc.Width();
	};
	int GetHeight() {
		return m_rc.Height();
	};
};

#endif