#ifndef test_opencv_match_h
#define test_opencv_match_h

#include <opencv/highgui.h>
#include <opencv/cv.h>

CvSeq* getSeq(char* path)
{
	IplImage* gun = cvLoadImage(path);

	IplImage * grey = cvCreateImage(cvGetSize(gun), IPL_DEPTH_8U, 1);//转灰度

	cvCvtColor(gun, grey, CV_RGB2GRAY);//转灰度

	IplImage * can = cvCreateImage(cvGetSize(gun), IPL_DEPTH_8U, 1);

	cvCanny(grey, can, 100, 200);

	CvMemStorage *mem = cvCreateMemStorage();

	CvSeq * con;

	cvFindContours(can, mem, &con, sizeof(CvContour),CV_RETR_EXTERNAL);
	/*
	printf("%d\n", cvFindContours(can, mem, &con, sizeof(CvContour)));

	IplImage* gunpic = cvCreateImage(cvGetSize(gun), IPL_DEPTH_8U, 3);

	cvDrawContours(gunpic, con, CV_RGB(255, 0, 0), CV_RGB(255, 0, 0), 1);

	cvNamedWindow("res", CV_WINDOW_AUTOSIZE);

	cvShowImage("res", gunpic);
	*/
	return con;
}

double match(CvSeq* con, char* path)
{
	if (path == NULL)
	{
		printf("NULL path!\n");
		return -1;
	}
	CvSeq * gc = getSeq(path);
	return cvMatchShapes(gc, con, CV_CONTOURS_MATCH_I1);
}

double testhu( int method)
{
	char gunPath[100] = "C:/Users/Clover/Desktop/gun.png";
	char knifePath[100] = "C:/Users/Clover/Desktop/knife.png";
	char hummerPath[100] = "C:/Users/Clover/Desktop/hummer.png";
	

	CvSeq * gc = getSeq(gunPath);
	CvSeq * kc = getSeq(knifePath);
	CvSeq * hc = getSeq(hummerPath);

	double gk, gh, kh;
	gk = cvMatchShapes(gc, kc, method);
	gh = cvMatchShapes(gc, hc, method);
	kh = cvMatchShapes(hc, kc, method);

	printf("gk=%lf gh=%lf kh=%lf", gk, gh, kh);

	cvWaitKey(0);




	return 0;
}



#endif