// CLiveDialog.cpp: 实现文件
//

#include "stdafx.h"
#include "startrtcdemo.h"
#include "CLiveDialog.h"
#include "afxdialogex.h"
#include "CLiveVideoVDN.h"
#include "CLiveVideoSrc.h"
#include <fstream>
BOOL SaveBitmapToFile(HBITMAP   hBitmap, CString szfilename);

// CLiveDialog 对话框

IMPLEMENT_DYNAMIC(CLiveDialog, CDialogEx)


CLiveDialog::CLiveDialog(CUserManager* pUserManager, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_SHOW_LIVE, pParent)
{
	m_pStartRtcLive = new CLiveVideoVDN(this, pUserManager);
	m_nShowWidth = 640;
	m_nShowHeight = 640;
}

CLiveDialog::~CLiveDialog()
{
	if (m_pStartRtcLive != NULL)
	{
		delete m_pStartRtcLive;
		m_pStartRtcLive = NULL;
	}
}

void CLiveDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_LIVVE_PICTURE_CONTROL, m_picControl);
}


BEGIN_MESSAGE_MAP(CLiveDialog, CDialogEx)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CLiveDialog 消息处理程序
void CLiveDialog::showBitmap(HBITMAP hBitmap)
{
	m_picControl.SetBitmap(hBitmap);
}
static long int crv_tab[256];
static long int cbu_tab[256];
static long int cgu_tab[256];
static long int cgv_tab[256];
static long int tab_76309[256];
static unsigned char clp[1024];   //for clip in CCIR601   

void init_yuv420p_table()
{
	long int crv, cbu, cgu, cgv;
	int i, ind;
	static int init = 0;

	if (init == 1) return;

	crv = 104597; cbu = 132201;  /* fra matrise i global.h */
	cgu = 25675;  cgv = 53279;

	for (i = 0; i < 256; i++)
	{
		crv_tab[i] = (i - 128) * crv;
		cbu_tab[i] = (i - 128) * cbu;
		cgu_tab[i] = (i - 128) * cgu;
		cgv_tab[i] = (i - 128) * cgv;
		tab_76309[i] = 76309 * (i - 16);
	}

	for (i = 0; i < 384; i++)
		clp[i] = 0;
	ind = 384;
	for (i = 0; i < 256; i++)
		clp[ind++] = i;
	ind = 640;
	for (i = 0; i < 384; i++)
		clp[ind++] = 255;

	init = 1;
}

/**
内存分布
					w
			+--------------------+
			|Y0Y1Y2Y3...         |
			|...                 |   h
			|...                 |
			|                    |
			+--------------------+
			|U0U1      |
			|...       |   h/2
			|...       |
			|          |
			+----------+
			|V0V1      |
			|...       |  h/2
			|...       |
			|          |
			+----------+
				w/2
 */
void yuv420p_to_rgb24(unsigned char* yuvbuffer, unsigned char* rgbbuffer, int width, int height)
{
	int y1, y2, u, v;
	unsigned char *py1, *py2;
	int i, j, c1, c2, c3, c4;
	unsigned char *d1, *d2;
	unsigned char *src_u, *src_v;
	static int init_yuv420p = 0;

	src_u = yuvbuffer + width * height;   // u
	src_v = src_u + width * height / 4;  //  v

	//if (type == FMT_YV12)
	//{
		src_v = yuvbuffer + width * height;   // v
		src_u = src_u + width * height / 4;  //  u
	//}
	py1 = yuvbuffer;   // y
	py2 = py1 + width;
	d1 = rgbbuffer;
	d2 = d1 + 3 * width;

	init_yuv420p_table();

	for (j = 0; j < height; j += 2)
	{
		for (i = 0; i < width; i += 2)
		{
			u = *src_u++;
			v = *src_v++;

			c1 = crv_tab[v];
			c2 = cgu_tab[u];
			c3 = cgv_tab[v];
			c4 = cbu_tab[u];

			//up-left   
			y1 = tab_76309[*py1++];
			*d1++ = clp[384 + ((y1 + c1) >> 16)];
			*d1++ = clp[384 + ((y1 - c2 - c3) >> 16)];
			*d1++ = clp[384 + ((y1 + c4) >> 16)];

			//down-left   
			y2 = tab_76309[*py2++];
			*d2++ = clp[384 + ((y2 + c1) >> 16)];
			*d2++ = clp[384 + ((y2 - c2 - c3) >> 16)];
			*d2++ = clp[384 + ((y2 + c4) >> 16)];

			//up-right   
			y1 = tab_76309[*py1++];
			*d1++ = clp[384 + ((y1 + c1) >> 16)];
			*d1++ = clp[384 + ((y1 - c2 - c3) >> 16)];
			*d1++ = clp[384 + ((y1 + c4) >> 16)];

			//down-right   
			y2 = tab_76309[*py2++];
			*d2++ = clp[384 + ((y2 + c1) >> 16)];
			*d2++ = clp[384 + ((y2 - c2 - c3) >> 16)];
			*d2++ = clp[384 + ((y2 + c4) >> 16)];
		}
		d1 += 3 * width;
		d2 += 3 * width;
		py1 += width;
		py2 += width;
	}
}

string byteToHexStr(unsigned char byte_arr[], int arr_len)

{

	string hexstr = "";

	for (int i = 0; i < arr_len; i++)
	{

		char hex1;

		char hex2;

		/*借助C++支持的unsigned和int的强制转换，把unsigned char赋值给int的值，那么系统就会自动完成强制转换*/

		int value = byte_arr[i];

		int S = value / 16;

		int Y = value % 16;

		//将C++中unsigned char和int的强制转换得到的商转成字母

		if (S >= 0 && S <= 9)

			hex1 = (char)(48 + S);

		else

			hex1 = (char)(55 + S);

		//将C++中unsigned char和int的强制转换得到的余数转成字母

		if (Y >= 0 && Y <= 9)

			hex2 = (char)(48 + Y);

		else

			hex2 = (char)(55 + Y);

		//最后一步的代码实现，将所得到的两个字母连接成字符串达到目的

		hexstr = hexstr + hex1 + hex2;

	}

	return hexstr;

}
void CLiveDialog::revData(int upid,int w, int h, uint8_t* videoData, int videoDataLen)
{
	//try
	{
		if (videoData != NULL && videoDataLen > 0)
		{
			string strTest = byteToHexStr(videoData, videoDataLen/5);
			/*static int nIndex = 0;
			CString path = "";
			path.Format("test/test%d.yuv", nIndex);
			//打开文件

			int arr[5] = { 1, 2, 3, 4, 5 };
			int i;
			for (i = 0; i < videoDataLen; i++)
			{
				//写入数据
				outFile << videoData[i];
			}
			//关闭文件
			//outFile.close();
			nIndex++;
			if (w != 640)
			{
				CString strW = "";
				strW.Format("w:%d", w);
				AfxMessageBox(strW);
			}*/
			uint8_t* videoDataRGB = new uint8_t[w*h * 3];

			yuv420p_to_rgb24(videoData, videoDataRGB, w, h);

			HBITMAP bitmap = CreateBitmap(w, h, 1, 1, videoDataRGB);

			CRect rect;
			
			m_picControl.GetClientRect(&rect);     //m_picture为Picture Control控件变量，获得控件的区域对象
			CImage image;       //使用图片类
			image.Create(w, h, 24, 0);
			//首地址  
			byte* pRealData = (byte*)image.GetBits();
			//首地址    
			//行距  
			int pit = image.GetPitch();
			for (int i = 0; i < w; i++)
			{
				for (int j = 0; j < h; j++)
				{
					*(pRealData + pit * j + i * 3) = (int)videoDataRGB[3 * j*w + 3 * i];
					*(pRealData + pit * j + i * 3 + 1) = (int)videoDataRGB[3 * j*w + 3 * i + 1];
					*(pRealData + pit * j + i * 3 + 2) = (int)videoDataRGB[3 * j*w + 3 * i + 2];
				}
			}		
			CDC* pDC = m_picControl.GetWindowDC();    //获得显示控件的DC
			pDC->SetStretchBltMode(COLORONCOLOR);
			image.Draw(pDC->m_hDC, rect);      //图片类的图片绘制Draw函数
			ReleaseDC(pDC);
			delete[] videoDataRGB;
			videoDataRGB = NULL;
		}
	}
	//catch (char *str)
	{
		printf("err");
	}
	
}



BOOL CLiveDialog::OnInitDialog()
{
	__super::OnInitDialog();
	CRect rect;
	GetClientRect(rect);
	int x = 0;
	int y = 0;
	int w = rect.Width();
	int h = rect.Height();

	float wScale = w/m_nShowWidth;
	float hScale = h/m_nShowHeight;
	if (m_nShowWidth > 0)
	{
		wScale = ((float)w) / ((float)m_nShowWidth);
	}

	if (m_nShowHeight > 0)
	{
		hScale = ((float)h) / ((float)m_nShowHeight);
		if (wScale < hScale)
		{
			h = m_nShowHeight * wScale;
			w = m_nShowWidth * wScale;
		}
		else
		{
			w = m_nShowWidth * hScale;

			h = m_nShowHeight * hScale;

		}
	}

	::MoveWindow(m_picControl.m_hWnd, (rect.Width()-w)/2, (rect.Height() - h) / 2,w,h,TRUE);
	if (m_pStartRtcLive != NULL)
	{
		m_pStartRtcLive->startRecvLiveData();
	}

	
	
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CLiveDialog::OnClose()
{
	if (m_pStartRtcLive != NULL)
	{
		m_pStartRtcLive->exit();
	}

	__super::OnClose();
}
