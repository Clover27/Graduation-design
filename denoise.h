//
//  Header.h
//  test_opencv
//
//  Created by 晟昱 陈 on 15/7/23.
//  Copyright (c) 2015年 晟昱 陈. All rights reserved.
//

#ifndef test_opencv_denoise_h
#define test_opencv_denoise_h


IplImage* denoise2(IplImage* img_src, double minarea)
{
	unsigned char * ppp;
	CvSeq* contour = NULL;
	double tmparea = 0.0;
	CvMemStorage* storage = cvCreateMemStorage(0);
	IplImage* img_Clone = cvCloneImage(img_src);
	//访问二值图像每个点的值

	//------------搜索二值图中的轮廓，并从轮廓树中删除面积小于某个阈值minarea的轮廓-------------//
	CvContourScanner scanner = NULL;
	scanner = cvStartFindContours(img_src, storage, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE, cvPoint(0, 0));
	//开始遍历轮廓树
	CvRect rect;
	while ((contour = cvFindNextContour(scanner)))
	{
		//tmparea = fabs(cvContourArea(contour));
		rect = cvBoundingRect(contour, 0);
		tmparea = rect.width*rect.height;
		if (tmparea < minarea/*||tmparea>4900*/)
		{
			if ((tmparea - 0.0) < 0.01) {
				CvSeq* hn = contour->h_next;
				CvSeq* hp = contour->h_prev;
				CvSeq* vn = contour->v_next;
				CvSeq* vp = contour->v_prev;
				if (hp != NULL)

					hp->h_next = hn;
				if (hn != NULL)
					hn->h_prev = hp;
				if (vp != NULL)
					vp->v_next = vn;
				if (vn != NULL)
					vn->v_prev = vp;

				for (int y = rect.y; y < rect.y + rect.height; y++)
				{
					for (int x = rect.x; x < rect.x + rect.width; x++)
					{
						ppp = (uchar*)(img_Clone->imageData + img_Clone->widthStep*y + x);

						if (ppp[0] == 0)
						{
							ppp[0] = 255;
						}
					}
				}

				continue;
			}
			//当连通域的中心点为黑色时，而且面积较小则用白色进行填充
			ppp = (uchar*)(img_Clone->imageData + img_Clone->widthStep*(rect.y + rect.height / 2) + rect.x + rect.width / 2);
			if (ppp[0] != 255)
			{
				for (int y = rect.y; y < rect.y + rect.height; y++)
				{
					for (int x = rect.x; x < rect.x + rect.width; x++)
					{
						ppp = (uchar*)(img_Clone->imageData + img_Clone->widthStep*y + x);

						if (ppp[0] == 0)
						{
							ppp[0] = 255;
						}
					}
				}
			}

		}
	}
	cvReleaseMemStorage(&storage);
	return img_Clone;
}
#endif

