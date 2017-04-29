/*
*渲染装置
*/
#ifndef DEVICE_H
#define DEVICE_H
#include "3DMath.h"
#include "Camera.h"
#include "GeometryDefine .h"

const int RENDER_STATE_WIREFRAME = 1;	// 渲染线框
const int RENDER_STATE_COLOR = 2;		// 渲染颜色

class Device
{
public:
	//渲染主函数
	void Render()
	{
		Clear();
		/*Point2D p1 = { -30, 250 };
		Point2D p2 = { 110, 350 };
		Point2D p3 = { 200, -220 };
		SetClip(60, 70, 500, 500);
		DrawTriangle(p1, p2, p3, foreground);*/

		//正方体
		Point3D mesh[8] = {
			{ 100, -100, 100, 1},
			{-100, -100, 100, 1},
			{-100,  100, 100, 1},
			{ 100,  100, 100, 1},
			{ 100, -100, -100, 1},
			{-100, -100, -100, 1},
			{-100,  100, -100, 1},
			{ 100,  100, -100, 1},
		};

		Triangle t1(mesh[0], mesh[1], mesh[2]);
		Triangle t2(mesh[2], mesh[3], mesh[1]);

		Triangle t3(mesh[4], mesh[5], mesh[6]);
		Triangle t4(mesh[6], mesh[7], mesh[4]);

		Triangle t5(mesh[0], mesh[4], mesh[7]);
		Triangle t6(mesh[7], mesh[3], mesh[0 ]);

		Triangle t7(mesh[1], mesh[5], mesh[6]);
		Triangle t8(mesh[6], mesh[2], mesh[1]);

		Triangle t9(mesh[0], mesh[1], mesh[5]);
		Triangle t10(mesh[5], mesh[4], mesh[0]);

		Triangle t11(mesh[3], mesh[2], mesh[6]);
		Triangle t12(mesh[6], mesh[7], mesh[3]);

		Triangle list[12] = { t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12 };

		Objecet *object = new Objecet({ 400,400,200,1 }, 12, list);
		AddObjectList(object);
		LocalToWorld();
		WorldToCamera();
		Projection();
		RenderObject();
		delete object;
		ClearObjectList();



	}
	void Clear()
	{
		memset(frameBuffer, 0, sizeof(UINT)* width * height);
	}
public:
	Device(UVNCamera *camera, int width, int height, void *frameBuffer, int renderState, UINT foreground, UINT background)
	{
		Init(camera, width, height, frameBuffer, renderState, foreground, background);
	}
	~Device()
	{

	}
	//设置裁剪区域
	void SetClip(int clipMinX, int clipMinY, int clipMaxX, int clipMaxY)
	{
		this->clipMaxX = clipMaxX;
		this->clipMaxY = clipMaxY;
		this->clipMinX = clipMinX;
		this->clipMinY = clipMinY;
	}
	//画点
	void DrawPoint(int x, int y, UINT color)
	{
		if (x < clipMinX || x >= clipMaxX || y < clipMinY || y >= clipMaxY)
		{
			return;
		}
		frameBuffer[(UINT)x + (UINT)y * (UINT)width] = color;
	}
	//画线
	void DrawLine(int x1, int y1, int x2, int y2, UINT color)
	{
		int dy = y2 - y1;
		int dx = x2 - x1;
		int x = x1;
		int y = y1;
		if (dx == 0)
		{
			if (y <= y2)
			{
				while (y <= y2)
				{
					DrawPoint(x, y++, color);
				}
			}
			else
			{
				while (y > y2)
				{
					DrawPoint(x, y--, color);
				}
			}
			return;
		}
		float k = (float)dy / (float)dx;
		float b = y1 - k*x1;
		if (abs(k) < 1)
		{
			if (x <= x2)
			{
				while (x <= x2)
				{
					DrawPoint(x, y, color);
					y = (int)(k * ++x + b + 0.5);
				}
			}
			else
			{
				while (x > x2)
				{
					DrawPoint(x, y, color);
					y = (int)(k * --x + b + 0.5);
				}
			}
		}
		else
		{
			if (y <= y2)
			{
				while (y <= y2)
				{
					DrawPoint(x, y, color);
					x = (int)((++y - b) / k + 0.5);
				}
			}
			else
			{
				while (y > y2)
				{
					DrawPoint(x, y, color);
					x = (int)((--y - b) / k + 0.5);
				}
			}
		}
	}
	void DrawLine(const Point2D &p1, const Point2D &p2, UINT color)
	{
		DrawLine(p1.x, p1.y, p2.x, p2.y, color);
	}
	//画三角形
	void DrawTriangle(const Point2D &p1, const Point2D &p2, const Point2D &p3, UINT color)
	{
		if (RENDER_STATE_WIREFRAME == renderState)
		{
			DrawLine(p1, p2, color);
			DrawLine(p2, p3, color);
			DrawLine(p3, p1, color);
		}
		else if (RENDER_STATE_COLOR == renderState)
		{
			//是否是平底或平顶三角形
			if (p1.y == p2.y)
			{
				if (p3.y > p2.y)
				{
					FillBottomTriangle(p1, p2, p3, color);
				}
				else if (p3.y < p2.y)
				{
					FillTopTriangle(p1, p2, p3, color);
				}
			}
			else if (p1.y == p3.y)
			{
				if (p2.y > p1.y)
				{
					FillBottomTriangle(p1, p2, p3, color);
				}
				else if (p2.y < p1.y)
				{
					FillTopTriangle(p1, p2, p3, color);
				}
			}
			else if (p2.y == p3.y)
			{
				if (p3.y > p2.y)
				{
					FillBottomTriangle(p1, p2, p3, color);
				}
				else if (p3.y < p2.y)
				{
					FillTopTriangle(p1, p2, p3, color);
				}
			}
			//不是平底也不是平顶,需要拆分
			else
			{
				//从上到下排列顶点
				Point2D pList[] = { p1, p2, p3 };
				SelectionSort(pList, 3);
				//(y - b) / k
				float k = (pList[0].y - pList[2].y) * 1.0f / (pList[0].x - pList[2].x);
				float b = pList[2].y - k*pList[2].x;
				int CenterPointX = (int)((pList[1].y - b) / k + 0.5);
				Point2D centerPoint = { CenterPointX, pList[1].y };
				FillTopTriangle(pList[0], centerPoint, pList[1], color);
				FillBottomTriangle(centerPoint, pList[1], pList[2], color);
			}
		}
	}
	void DrawTriangle(const Triangle &triangle, UINT color)
	{
		Point2D p1 = { (int)triangle.newPos[0].x, (int)triangle.newPos[0].y };
		Point2D p2 = { (int)triangle.newPos[1].x, (int)triangle.newPos[1].y };
		Point2D p3 = { (int)triangle.newPos[2].x, (int)triangle.newPos[2].y };
		DrawTriangle(p1, p2, p3, color);
	}

	//填充平顶三角形
	void FillTopTriangle(const Point2D &p1, const Point2D &p2, const Point2D &p3, UINT color)
	{
		Point2D top, left, right;
		if (p1.y == p2.y)
		{
			top = p3;
			left = p1;
			right = p2;
		}
		else if (p2.y == p3.y)
		{
			top = p1;
			left = p2;
			right = p3;
		}
		else
		{
			top = p2;
			left = p1;
			right = p3;
		}
		if (left.x > right.x)
		{
			Swap(left, right);
		}
		float leftDxDivDy = ((float)top.x - (float)left.x) / ((float)top.y - (float)left.y);
		float rightDxDivDy = ((float)right.x - (float)top.x) / ((float)right.y - (float)top.y);
		float xleft = (float)left.x;
		float xRight = (float)right.x;
		int y = left.y;
		while (y >= top.y)
		{
			DrawLine((int)(xleft + 0.5), y, (int)(xRight + 0.5), y, color);
			y--;
			xleft -= leftDxDivDy;
			xRight -= rightDxDivDy;
		}
	}

	//填充平底三角形
	void FillBottomTriangle(const Point2D &p1, const Point2D &p2, const Point2D &p3, UINT color)
	{
		Point2D bottom, left, right;
		if (p1.y == p2.y)
		{
			bottom = p3;
			left = p1;
			right = p2;
		}
		else if (p2.y == p3.y)
		{
			bottom = p1;
			left = p2;
			right = p3;
		}
		else
		{
			bottom = p2;
			left = p1;
			right = p3;
		}
		if (left.x > right.x)
		{
			Swap(left, right);
		}

		float leftDxDivDy = ((float)bottom.x - (float)left.x) / ((float)bottom.y - (float)left.y);
		float rightDxDivDy = ((float)right.x - (float)bottom.x) / ((float)right.y - (float)bottom.y);
		float xleft = (float)left.x;
		float xRight = (float)right.x;
		int y = left.y;
		while (y <= bottom.y)
		{
			DrawLine((int)(xleft + 0.5), y, (int)(xRight + 0.5), y, color);
			y++;
			xleft += leftDxDivDy;
			xRight += rightDxDivDy;
		}
	}

	//将物体加入渲染列表
	void AddObjectList(Objecet *objecet)
	{
		if (objecet == nullptr)
		{
			return;
		}
		objecetList[objectListCount++] = objecet;
	}

	//渲染物体列表
	void RenderObject()
	{
		for (int i = 0; i < objectListCount; i++)
		{
			for (int j = 0; j < objecetList[i]->triangleCount; j++)
			{
				DrawTriangle(objecetList[i]->triangleList[j], foreground);
			}
		}
	}

	//清空渲染列表
	void ClearObjectList()
	{
		objectListCount = 0;
	}

	//相机变换
	void WorldToCamera()
	{
		for (int i = 0; i < objectListCount; i++)
		{
			for (int j = 0; j < objecetList[i]->triangleCount; j++)
			{
				for (int k = 0; k < 3; k++)
				{
					objecetList[i]->triangleList[j].newPos[k] = camera->WolrdToCamera(objecetList[i]->triangleList[j].newPos[k]);
				}
			}
		}
	}

	//世界变换
	void LocalToWorld()
	{
		for (int i = 0; i < objectListCount; i++)
		{
			Matrix4X4 translation = { 1,0,0,0,
				0,1,0,0,
				0,0,1,0,
				objecetList[i]->position.x, objecetList[i]->position.y, objecetList[i]->position.z, 1 };

			for (int j = 0; j < objecetList[i]->triangleCount; j++)
			{
				for (int k = 0; k < 3; k++)
				{
					MatrixApply(objecetList[i]->triangleList[j].newPos[k], 
						objecetList[i]->triangleList[j].oldPos[k], translation);
				}
			}
		}
	}

	//投影变换
	void Projection()
	{
		for (int i = 0; i < objectListCount; i++)
		{
			for (int j = 0; j < objecetList[i]->triangleCount; j++)
			{
				for (int k = 0; k < 3; k++)
				{
					objecetList[i]->triangleList[j].newPos[k].x = objecetList[i]->triangleList[j].newPos[k].x / objecetList[i]->triangleList[j].newPos[k].z * camera->GetViewDistance();
					objecetList[i]->triangleList[j].newPos[k].y = objecetList[i]->triangleList[j].newPos[k].y / objecetList[i]->triangleList[j].newPos[k].z * camera->GetViewDistance();
				}
			}
		}
	}

private:
	void Init(UVNCamera *camera, int width, int height, void *frameBuffer, int renderState, UINT foreground, UINT background)
	{
		this->camera = camera;
		this->width = width;
		this->height = height;
		this->frameBuffer = (UINT*)frameBuffer;
		this->renderState = renderState;
		this->foreground = foreground;
		this->background = background;
		this->clipMinX = 0;
		this->clipMinY = 0;
		this->clipMaxX = width;
		this->clipMaxY = height;
		objectListCount = 0;
	}
	void Destroy()
	{
		if (frameBuffer != nullptr)
		{
			delete frameBuffer;
			frameBuffer = nullptr;
		}
	}
private:
	int width;
	int height;
	UINT *frameBuffer;		//图像缓存
	int renderState;
	UINT background;		//背景颜色
	UINT foreground;		//线框颜色
	int clipMaxX;
	int clipMaxY;
	int clipMinX;
	int clipMinY;

	UVNCamera *camera;		//相机

	int objectListCount;
	Objecet *objecetList[64];			//渲染物体列表
};


#endif
