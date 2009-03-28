#ifndef __KIT_EDITOR_H__
#define __KIT_EDITOR_H__

#include <QtGui>
#include <imgkit.h>
#include <alloca.h>

class GbcConv
{
public:
	u32 m_iBlkCount;

	u32 m_iChrCount; // Number of unique characters in ChrData
	u8 *m_pChrData;  // Character data

	u32 m_iPalCount; // Number of unique palettes in PalData
	u16 *m_pPalData;  // Palette data

	int *m_pBlkPal; // which palette each block uses 
	int *m_pBlkChr; // which character each block uses 
	int *m_pBlkErr; 


	RasterSurface *m_pSurface; 

public:
	GbcConv(RasterSurface *surface);
	~GbcConv();
	int findChr(u8 *thisChr);
	int findPal(u16 *colours, bool);
	void imageResetErrors() {memset(m_pBlkErr, 0, sizeof(u32)*m_iBlkCount);}

	int readPal(const char *filename);
	int writePal(const char *filename, const char *name);
	int writeChr(const char *filename, const char *name);
	int writeMap(const char *filename, const char *name);

	int imageDoPal(bool generate=true);
	int imageDoChr();
};

class EditorWindow;

class EditorShow : public QLabel
{
public:
	EditorShow(EditorWindow *foo) : QLabel()  { hax = foo; }
private:
	EditorWindow *hax;
	void mouseMoveEvent ( QMouseEvent * event ) ;
	void mousePressEvent ( QMouseEvent * event ) ;
};

class EditorWindow : public QWidget
{
	Q_OBJECT

public:
	EditorWindow(const char *afilename);
	void drawStuff();
	void mouse (int x, int y);

private:


	int curBlk,curBlkX, curBlkY;
	int width, height;
	GbcConv *conv;
	QImage ourImage;
	QImage resultImage;
    EditorShow *resultLabel;
	QLabel *blockInfo;
	QTimer *timer;
	bool m_bShowErrors;
	QLineEdit *namebox;
	QPushButton *reload;
	QPushButton *loadPal;
	QPushButton *saveSet;
	RasterSurface surface; 


	const char *origFilename;

private slots:
	void reloadConv();
	void saveStuff();
	void loadPalFunc();
    void timerTick();
};



#endif /* __KIT_EDITOR_H__ */

