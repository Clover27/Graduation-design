#include <opencv/highgui.h>
#include <opencv/cv.h>
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <iostream>

#include "denoise.h"
#include "match.h"

using namespace std;

CvSeq* zoomoutC(CvSeq*, double);

IplImage* doCanny(IplImage* in, double lowThresh, double highThresh, double aperture)
{
	if (in->nChannels != 1)
	{
		printf("in do Canny: channels!=1\n");
		return 0;
	}
	IplImage * out = cvCreateImage(cvGetSize(in), IPL_DEPTH_8U, 1);

	cvCanny(in, out, (double)lowThresh, (double)highThresh, (double)aperture);
	return out;
}


//传入灰度或二值图像

IplImage* ScanAndDraw(IplImage* img, double maxarea = 3000, IplImage* body = NULL, CvSeq** ob = NULL, int low = 100, int high = 200)
{
	bool isBody = false;
	CvContourScanner scanner = NULL;
	//检测body尺寸

	if (body != NULL)
	{
		if (body->width != img->width || body->height != img->height || body->nChannels < 3)
		{
			if (body->nChannels != 3)
				printf("Invalid input body image!\n");
			//缩放

			else
			{
				CvSize sz;
				sz.height = img->height;
				sz.width = img->width;

				IplImage* tmp = cvCreateImage(sz, body->depth, 3);

				cvResize(body, tmp, CV_INTER_AREA);

				body = tmp;

				isBody = 1;
			}


			//return NULL;
		}
		else
		{
			isBody = true;
			printf("body = 1 \n");
		}


	}

	IplImage* out3 = doCanny(img, low, high, 3);

	CvMemStorage *mem = cvCreateMemStorage();

	IplImage* co1 = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 3);
	cvSetZero(co1);

	//画

	scanner = cvStartFindContours(out3, mem, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE, cvPoint(0, 0));
	//开始遍历轮廓树
	int current = 0;
	/*if (ob != NULL)
	{
	ob = new CvSeq*[100];
	}*/

	CvRect rect;
	CvSeq* contour = NULL;

	while ((contour = cvFindNextContour(scanner)))
	{
		rect = cvBoundingRect(contour, 0);
		double tmparea = rect.height*rect.width;
		rect = cvBoundingRect(contour, 0);
		bool f = 1;
		int ry;
		ry = rect.y + rect.height / 2;
		if (ry<img->height / 3)
			f = 0;
		if (tmparea < maxarea
			&& tmparea > 40 && rect.width>5
			&& rect.height > 5 && f)
		{
			printf("%lf +1\n",tmparea);
			cvDrawContours(co1, contour, CV_RGB(255, 100, 0), CV_RGB(255, 100, 0), 0, 2);


			if (ob != NULL)
			{
				ob[current] = cvCloneSeq(contour);
				ob[current]->h_next = NULL;
				if (current != 0)
				{
					ob[current - 1]->h_next = ob[current];
				}
				current++;
				//printf("-1\n");
			}
			//debug
			//cvEllipseBox(co1, cvMinAreaRect2(contour), CV_RGB(255, 255, 255), 1);
			//cvRectangleR(co1,cvBoundingRect(contour),CV_RGB(255,255,255),1);
			//

			if (isBody)
				cvDrawContours(body, contour, CV_RGB(255, 100, 0), CV_RGB(255, 100, 0), 0, 2);
		}
		else
		{
			CvSeq* ac = cvApproxPoly(contour, sizeof(CvContour), mem, CV_POLY_APPROX_DP, 3); //身体

			//画人体外圈

			/*
			if (!isBody)
			{
			CvSeq* out = zoomoutC(ac, 1.1);
			cvDrawContours(co1, out, CV_RGB(160, 32, 240), CV_RGB(160, 32, 240), 0, 2);
			}
			*/

			//画人体

			cvDrawContours(co1, ac, CV_RGB(10, 100, 200), CV_RGB(10, 100, 200), 0, 2);
			/////////////查找轮廓凹陷-----------

			vector<CvSeq*> aa;


			for (int i = 1; i < ac->total; i++)//
			{
				CvPoint a0 = *(CvPoint*)cvGetSeqElem(ac, i - 1);
				CvPoint a1 = *(CvPoint*)cvGetSeqElem(ac, i);
				CvPoint a2 = *(CvPoint*)cvGetSeqElem(ac, i + 1);

				if (a0.y<co1->height / 3
					//||a0.y> rect.height*2/3

					|| a2.y<co1->height / 3
					//||a2.y>rect.height*2/3
					//|| (a1.x>rect.width*3/7 && a1.x<rect.width*4/7) //排除中心线
					//|| (a1.x>rect.width*3/7 && a2.x<rect.width*4/7) //排除中心线

					|| a0.y>co1->height * 3 / 4 || a2.y>co1->height * 3 / 4

					)
				{
					continue;
				}


				int dx10 = a1.x - a0.x;
				int dx21 = a2.x - a1.x;
				int dy10 = a1.y - a0.y;
				int dy21 = a2.y - a1.y;

				if ((dx10* dx21 <= 0 || dy10*dy21 <= 0)
					//&& (abs((double)dx10/(double)dy10)-abs((double)dx21/(double)dy21))/abs((double)dx10/(double)dy10) > 0.3

					&& (dy21 != 0 && dy10 != 0 && ((abs(dx10 / dy10) > 0.7) || abs(dx21 / dy21) > 0.7))
					)
				{
					CvSeq* ao = cvCreateSeq(CV_SEQ_POINT_SET | CV_SEQ_FLAG_CLOSED, sizeof(CvSeq), sizeof(CvPoint), mem);
					cvSeqPush(ao, &a0);
					cvSeqPush(ao, &a1);
					cvSeqPush(ao, &a2);
					//printf("%lf",(abs((double)dx10/(double)dy10)-abs((double)dx21/(double)dy21))/abs((double)dx10/(double)dy10));

					cvCircle(co1, a0, 2, CV_RGB(255, 255, 255));
					cvCircle(co1, a1, 2, CV_RGB(255, 255, 255));
					cvCircle(co1, a2, 2, CV_RGB(255, 255, 255));
					//printf("(%d) (%d)\n",a0.x,a0.y);
					//printf("(%d) (%d)\n",a1.x,a1.y);
					//printf("(%d) (%d)\n",a2.x,a2 .y);

					aa.push_back(ao);
				}


			}

			CvSeq* pre = NULL;
			for (int i = 0; i < aa.size(); i++)
			{
				CvPoint ap;
				CvPoint ap2;

				if (pre != NULL)
				{
					ap = *(CvPoint*)cvGetSeqElem(pre, pre->total - 1);
					ap2 = *(CvPoint*)cvGetSeqElem(pre, pre->total - 2);
				}
				CvSeq* ao = aa.at(i);
				CvPoint a0 = *(CvPoint*)cvGetSeqElem(ao, 0);
				if (pre != NULL && ((abs(ap.y - a0.y) < 10 && abs(ap.x - a0.x) < 10)
					|| (abs(ap2.y - a0.y) < 10 && abs(ap2.x - a0.x) < 10)
					)

					&& cvBoundingRect(pre).height*cvBoundingRect(pre).width < 1000 && cvBoundingRect(pre).width < 50 && cvBoundingRect(pre).height < 30
					)
				{
					CvPoint a1 = *(CvPoint*)cvGetSeqElem(ao, 1);
					CvPoint a2 = *(CvPoint*)cvGetSeqElem(ao, 2);
					cvSeqPush(pre, &a0);
					cvSeqPush(pre, &a1);
					cvSeqPush(pre, &a2);
					continue;
				}
				else
				{
					if (pre == NULL)
					{
						pre = ao;
						continue;
					}
					CvRect ar = cvBoundingRect(pre);
					//if(ar.width*ar.height> 0 && ar.height <50 && ar.width>15 && ar.y+ar.height < co1->height-40
					//&& !((ar.x>rect.width*2/5 && ar.x<rect.width*3/5) //排除中心线
					//|| (ar.x+ar.width>rect.width*2/5 && ar.x+ar.width<rect.width*3/5))
					//   ) ///

					if (ar.width*ar.height > 200 && ar.height < 50
						//&& ar.width>15

						)

					{
						cvRectangleR(co1, ar, CV_RGB(0, 255, 0), 1);
						if (isBody)
							cvRectangleR(body, ar, CV_RGB(0, 255, 0), 1);
					}
					else
					{
						cvRectangleR(co1, ar, CV_RGB(255, 255, 0));
						if (isBody)
							cvRectangleR(body, ar, CV_RGB(255, 255, 0));
					}
					pre = ao;

				}


			}
			if (pre != NULL)
			{
				CvRect ar = cvBoundingRect(pre);
				//if(ar.width*ar.height> 0 && ar.height <50 && ar.width>15 && ar.y+ar.height < co1->height-40) ///

				if (ar.width*ar.height > 200 && ar.height < 50
					//&& ar.width>15

					)
				{
					cvRectangleR(co1, ar, CV_RGB(0, 255, 0), 1);
					if (isBody)
						cvRectangleR(body, ar, CV_RGB(0, 255, 0), 1);
				}
				else
				{
					cvRectangleR(co1, ar, CV_RGB(255, 255, 0));
					if (isBody)
						cvRectangleR(body, ar, CV_RGB(255, 255, 0));
				}
			}



		}

	}
	//cvReleaseMemStorage(&mem);
	cvReleaseImage(&out3);


	printf("end!\n");
	if (isBody)
		return body;
	else
		return co1;

}

//画外圈

CvSeq* zoomoutC(CvSeq* in, double scale)
{
	//CvSeq* out = cvCreateSeq(in->flags, sizeof(CvSeq), in->elem_size, in->storage);
	//cvCopy(in, out);

	CvSeq* out = cvCloneSeq(in, in->storage);

	CvRect rec = cvBoundingRect(in);
	double hight = rec.height;
	double width = rec.width;

	double c_x = rec.x + (double)width / 2;
	double c_y = rec.y + (double)hight / 2;


	printf("%lf , %lf", c_x, c_y);

	for (int i = 0; i < out->total; i++)
	{
		CvPoint* p = (CvPoint*)cvGetSeqElem(out, i);
		if (p != NULL)
		{
			p->x = scale * (double)p->x + (1 - scale)*c_x;
			p->y = scale * (double)p->y + (1 - scale)*c_y;
		}

	}

	return out;
}



/*****主函数（返回IplImage结构指针）
参数:
imgpath:图像路径
thre:阈值化阈值
denoise_area:噪点外接矩形面积最大值
body_maxarea:人形外接矩形面积最小值
bodypath:卡通人形图像路径（为空时将返回原始图像的加强图像）

return:结果图像
*/
IplImage* scanImage(char* imgpath, CvSeq**ob = NULL, char* bodypath = NULL, int thre = 125, int denoise_area = 400, int body_maxarea = 3000)
{
	if (imgpath == NULL)
	{
		printf("imgpath = NULL!\n");
		return NULL;
	}
	if (thre <0 || thre >255)
	{
		printf("thre invalid!\n");
		return NULL;
	}
	IplImage * img = cvLoadImage(imgpath); //打开

	IplImage * grey = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 1);//转灰度

	cvCvtColor(img, grey, CV_RGB2GRAY);//转灰度

	IplImage* gt = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 1);//储存阈值化图像

	cvThreshold(grey, gt, thre, 255, CV_THRESH_BINARY); //阈值化

	IplImage* dimg2 = denoise2(gt, denoise_area); //降噪

	IplImage* b = cvCreateImage(cvSize(dimg2->width + 10, dimg2->height + 10), IPL_DEPTH_8U, 1);//储存加边

	cvCopyMakeBorder(dimg2, b, cvPoint(5, 5), IPL_BORDER_CONSTANT, CV_RGB(255, 255, 255));//加边

	IplImage* bto = cvCreateImage(cvGetSize(b), IPL_DEPTH_8U, 1);//储存开运算

	cvMorphologyEx(b, bto, NULL, NULL, CV_MOP_OPEN, 1);//开运算

	IplImage* body = NULL;

	if (bodypath != NULL && bodypath != "")
	{
		body = cvLoadImage(bodypath);
	}

	IplImage * btoc = ScanAndDraw(bto, body_maxarea, body, ob); //轮廓处理

	cvReleaseImage(&grey);
	cvReleaseImage(&gt);
	cvReleaseImage(&dimg2);
	cvReleaseImage(&img);
	cvReleaseImage(&b);
	cvReleaseImage(&bto);
	//cvReleaseImage(&body);
	return btoc;
}


//TODO add seq
/*****主函数（保存图像）
参数:
imgpath:图像路径
svpath:结果保存路径（含有图像后缀名）
thre:阈值化阈值
denoise_area:噪点外接矩形面积最大值
body_maxarea:人形外接矩形面积最小值
bodypath:卡通人形图像路径（为空时将保存原始图像的加强图像）

return:cvSaveImage的返回值，根据文件格式的不同返回非负数。如果运行失败返回负数
*/
int scanImageSv(char* imgpath, char* svpath, CvSeq** ob = NULL, char* bodypath = NULL, int thre = 125, int denoise_area = 400, int body_maxarea = 3000)
{
	if (imgpath == NULL || svpath == NULL)
	{
		printf("imgpath = NULL!\n");
		return -1;
	}
	if (thre <0 || thre >255)
	{
		printf("thre invalid!\n");
		return -2;
	}
	IplImage * img = cvLoadImage(imgpath); //打开

	IplImage * grey = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 1);//转灰度

	cvCvtColor(img, grey, CV_RGB2GRAY);//转灰度

	IplImage* gt = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 1);//储存阈值化图像

	cvThreshold(grey, gt, thre, 255, CV_THRESH_BINARY); //阈值化

	IplImage* dimg2 = denoise2(gt, denoise_area); //降噪

	IplImage* b = cvCreateImage(cvSize(dimg2->width + 10, dimg2->height + 10), IPL_DEPTH_8U, 1);//储存加边

	cvCopyMakeBorder(dimg2, b, cvPoint(5, 5), IPL_BORDER_CONSTANT, CV_RGB(255, 255, 255));//加边

	IplImage* bto = cvCreateImage(cvGetSize(b), IPL_DEPTH_8U, 1);//储存开运算

	cvMorphologyEx(b, bto, NULL, NULL, CV_MOP_OPEN, 1);//开运算

	IplImage* body = NULL;

	if (bodypath != NULL && bodypath != "")
	{
		body = cvLoadImage(bodypath);
	}



	IplImage * btoc = ScanAndDraw(bto, body_maxarea, body); //轮廓处理

	int res = cvSaveImage(svpath, btoc);//保存图像

	cvReleaseImage(&grey);
	cvReleaseImage(&gt);
	cvReleaseImage(&dimg2);
	cvReleaseImage(&img);
	cvReleaseImage(&b);
	cvReleaseImage(&bto);
	//cvReleaseImage(&body);

	cvReleaseImage(&btoc);


	return res;

}

int scan()
{
	char path[100];
	char bodypath[100] = "/Users/Clover/Desktop/body.jpg";

	for (int i = 101; i <= 103; i++) {
		sprintf(path, "C:/Users/Clover/Desktop/1/%d.bmp", i);

		IplImage* src = cvLoadImage(path);

		CvSeq** ob = new CvSeq*[100];
		ob[0] = NULL;

		//IplImage* res = scanImage(path,bodypath);

		IplImage * res = scanImage(path, ob, NULL);
		cvNamedWindow("res", CV_WINDOW_AUTOSIZE);
		cvNamedWindow("src", CV_WINDOW_AUTOSIZE);
		cvShowImage("res", res);
		cvShowImage("src", src);
		//cvSaveImage("/Users/Clover/Desktop/1/%d_res.bmp", res);

		CvSeq* current = ob[0];
		char gunPath[100] = "C:/Users/Clover/Desktop/gun1.png";
		char knifePath[100] = "C:/Users/Clover/Desktop/knife1.png";
		char hummerPath[100] = "C:/Users/Clover/Desktop/hummer1.png";
		while (current != NULL)
		{
			double g, k, h;
			g = match(current, gunPath);
			k = match(current, knifePath);
			h = match(current, hummerPath);


			printf("g = %lf, k = %lf, h = %lf\n", g, k, h);
			current = current->h_next;

		}



		cvWaitKey(0);
		cvReleaseImage(&res);
		cvReleaseImage(&src);
	}
	return 0;
}

int main()
{
	//return scan();

	//testhu(1);


	//scan();
	//cvWaitKey(0);

	scan();



	system("pause");
	return 0;
}





