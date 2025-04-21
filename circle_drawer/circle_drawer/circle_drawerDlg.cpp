#include "pch.h"
#include "framework.h"
#include "circle_drawer.h"
#include "circle_drawerDlg.h"
#include "afxdialogex.h"
#include <gdiplus.h>
#include <cmath>
#include <ctime>
#include <thread>
using namespace Gdiplus;
#pragma comment(lib, "Gdiplus.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class CAboutDlg : public CDialogEx {
public:
	CAboutDlg();
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif
protected:
	virtual void DoDataExchange(CDataExchange* pDX);
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX) {}
void CAboutDlg::DoDataExchange(CDataExchange* pDX) {
	CDialogEx::DoDataExchange(pDX);
}
BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

CcircledrawerDlg::CcircledrawerDlg(CWnd* pParent)
	: CDialogEx(IDD_CIRCLE_DRAWER_DIALOG, pParent),
	m_clickCount(0), m_selectedPoint(-1), m_dragging(false), m_radius(0.0f), m_hIcon(NULL), m_stopThread(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CcircledrawerDlg::DoDataExchange(CDataExchange* pDX) {
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CcircledrawerDlg, CDialogEx)
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_LBUTTONDOWN()
	ON_EN_CHANGE(IDC_EDIT_Radius, &CcircledrawerDlg::OnEnChangeEditRadius)
	ON_EN_CHANGE(IDC_EDIT_thickness, &CcircledrawerDlg::OnEnChangeEditRadius)
	ON_BN_CLICKED(IDC_BUTTON_clear, &CcircledrawerDlg::OnBnClickedButtonClear)
	ON_BN_CLICKED(IDC_BUTTON_random, &CcircledrawerDlg::OnBnClickedButtonRandom)
	ON_BN_CLICKED(IDC_BUTTON_start, &CcircledrawerDlg::OnBnClickedButtonStart)
END_MESSAGE_MAP()

BOOL CcircledrawerDlg::OnInitDialog() {
	CDialogEx::OnInitDialog();
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);
	return TRUE;
}

HCURSOR CcircledrawerDlg::OnQueryDragIcon() {
	return static_cast<HCURSOR>(m_hIcon);
}

void CcircledrawerDlg::OnSysCommand(UINT nID, LPARAM lParam) {
	CDialogEx::OnSysCommand(nID, lParam);
}

bool CcircledrawerDlg::GetCircleFrom3Points(CPoint p1, CPoint p2, CPoint p3, PointF& center, float& radius) {
	float x1 = (float)p1.x, y1 = (float)p1.y;
	float x2 = (float)p2.x, y2 = (float)p2.y;
	float x3 = (float)p3.x, y3 = (float)p3.y;
	float a = x1 * (y2 - y3) - y1 * (x2 - x3) + x2 * y3 - x3 * y2;
	if (fabs(a) < 1e-6) return false;
	float A1 = x1 * x1 + y1 * y1;
	float A2 = x2 * x2 + y2 * y2;
	float A3 = x3 * x3 + y3 * y3;
	float bx = (A1 * (y2 - y3) + A2 * (y3 - y1) + A3 * (y1 - y2)) / (2 * a);
	float by = (A1 * (x3 - x2) + A2 * (x1 - x3) + A3 * (x2 - x1)) / (2 * a);
	center.X = bx;
	center.Y = by;
	radius = sqrt((x1 - bx) * (x1 - bx) + (y1 - by) * (y1 - by));
	return true;
}

void CcircledrawerDlg::OnPaint() {
	CPaintDC dc(this);
	Graphics graphics(dc);
	CString radiusStr, thicknessStr;
	GetDlgItemText(IDC_EDIT_Radius, radiusStr);
	GetDlgItemText(IDC_EDIT_thickness, thicknessStr);
	float inputRadius = _ttof(radiusStr);
	float thickness = _ttof(thicknessStr);
	if (thickness <= 0) thickness = 2.0f;
	Pen pen(Color(255, 255, 0, 0), thickness);
	SolidBrush brush(Color(255, 0, 0));
	const WCHAR* labels[3] = { L"x", L"y", L"z" };

	if (m_clickCount >= 1 && m_clickCount <= 2) {
		for (int i = 0; i < m_clickCount; ++i) {
			dc.TextOutW(m_clickPoints[i].x + 8, m_clickPoints[i].y + 8, labels[i]);
			graphics.FillEllipse(&brush, RectF(m_clickPoints[i].x - 5.0f, m_clickPoints[i].y - 5.0f, 10.0f, 10.0f));
		}
	}

	if (m_clickCount == 3) {
		PointF center;
		float originalRadius;
		if (GetCircleFrom3Points(m_clickPoints[0], m_clickPoints[1], m_clickPoints[2], center, originalRadius)) {
			m_radius = (inputRadius > 0.0f) ? inputRadius : originalRadius;

			PointF scaledPoints[3];
			for (int i = 0; i < 3; ++i) {
				float dx = m_clickPoints[i].x - center.X;
				float dy = m_clickPoints[i].y - center.Y;
				float scale = (originalRadius == 0) ? 1.0f : (m_radius / originalRadius);
				scaledPoints[i].X = center.X + dx * scale;
				scaledPoints[i].Y = center.Y + dy * scale;
			}

			graphics.DrawEllipse(&pen, center.X - m_radius, center.Y - m_radius, m_radius * 2, m_radius * 2);
			for (int i = 0; i < 3; ++i) {
				graphics.FillEllipse(&brush, RectF(scaledPoints[i].X - 5.0f, scaledPoints[i].Y - 5.0f, 10.0f, 10.0f));
				dc.TextOutW((int)(scaledPoints[i].X + 8), (int)(scaledPoints[i].Y + 8), labels[i]);
			}
		}
	}
}

void CcircledrawerDlg::OnLButtonDown(UINT nFlags, CPoint point) {
	if (m_clickCount < 3) {
		m_clickPoints[m_clickCount++] = point;
	}
	else {
		for (int i = 0; i < 3; ++i) {
			int dx = point.x - m_clickPoints[i].x;
			int dy = point.y - m_clickPoints[i].y;
			if (sqrt(dx * dx + dy * dy) < 10) {
				m_selectedPoint = i;
				m_dragging = true;
				break;
			}
		}
	}
	Invalidate();
	CDialogEx::OnLButtonDown(nFlags, point);
}

void CcircledrawerDlg::OnMouseMove(UINT nFlags, CPoint point) {
	if (m_dragging && m_selectedPoint != -1) {
		m_clickPoints[m_selectedPoint] = point;
		Invalidate();
	}
	CDialogEx::OnMouseMove(nFlags, point);
}

void CcircledrawerDlg::OnLButtonUp(UINT nFlags, CPoint point) {
	m_dragging = false;
	m_selectedPoint = -1;
	CDialogEx::OnLButtonUp(nFlags, point);
}

void CcircledrawerDlg::OnDestroy() {
	m_stopThread = true;
	if (m_autoThread.joinable()) m_autoThread.join();
	GdiplusShutdown(m_gdiplusToken);
	CDialogEx::OnDestroy();
}

void CcircledrawerDlg::OnEnChangeEditRadius() {
	Invalidate();
}

void CcircledrawerDlg::OnBnClickedButtonClear() {
	m_clickCount = 0;
	m_selectedPoint = -1;
	m_dragging = false;
	m_radius = 0.0f;
	Invalidate();
}

void CcircledrawerDlg::OnBnClickedButtonRandom() {
	CRect rect;
	GetClientRect(&rect);
	int safeMargin = 150;
	srand((unsigned)time(NULL));
	for (int i = 0; i < 3; ++i) {
		m_clickPoints[i].x = rand() % (rect.Width() - safeMargin) + safeMargin / 2;
		m_clickPoints[i].y = rand() % (rect.Height() - safeMargin) + safeMargin / 2;
	}
	m_clickCount = 3;
	m_radius = static_cast<float>((rand() % 50) + 50);
	Invalidate();
}

void CcircledrawerDlg::OnBnClickedButtonStart() {
	if (m_autoThread.joinable()) m_autoThread.join();
	m_stopThread = false;
	m_autoThread = std::thread([this]() {
		for (int i = 0; i < 10 && !m_stopThread; ++i) {
			PostMessage(WM_COMMAND, MAKEWPARAM(IDC_BUTTON_random, BN_CLICKED), (LPARAM)GetDlgItem(IDC_BUTTON_random)->m_hWnd);
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
		});
}