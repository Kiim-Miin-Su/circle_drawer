#pragma once
#include <gdiplus.h>
#include <thread> // 멀티스레드를 위해 추가
using namespace Gdiplus;

class CcircledrawerDlg : public CDialogEx
{
private:
	ULONG_PTR m_gdiplusToken;
	CPoint m_clickPoints[3];
	int m_clickCount;
	int m_selectedPoint;
	bool m_dragging;
	float m_radius; // 사용자 입력 반지름
	HICON m_hIcon;  // 아이콘 핸들 추가
	std::thread m_autoThread; // 자동 랜덤 쓰레드
	bool m_threadRunning = false; // 쓰레드 실행 상태
	bool m_stopThread = false;    // 쓰레드 종료 요청 플래그

public:
	CcircledrawerDlg(CWnd* pParent = nullptr);

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CIRCLE_DRAWER_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnEnChangeEditRadius();
	afx_msg void OnBnClickedButtonClear();
	afx_msg void OnBnClickedButtonRandom();
	afx_msg void OnBnClickedButtonStart(); // 자동 실행 핸들러 선언 추가

	void AutoRandomThread(); // 쓰레드 동작 함수 선언

	DECLARE_MESSAGE_MAP()

private:
	bool GetCircleFrom3Points(CPoint p1, CPoint p2, CPoint p3, Gdiplus::PointF& center, float& radius);
};