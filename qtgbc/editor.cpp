
#include "editor.h"


#define ERROR_NOPAL 1
#define ERROR_MANYCOLORS 2


GbcConv::~GbcConv()
{
	delete[] m_pChrData;
	delete[] m_pPalData;
	delete[] m_pBlkPal;
	delete[] m_pBlkChr;
	delete[] m_pBlkErr; 
}

int GbcConv::writePal(const char *filename, const char *name)
{
	dinfo("%s: writing palettes (%s)", filename, name);
	FILE *f = fopen(filename, "w");
	if (f==NULL)
	{
		dwarn("%s: failed to open (%s)", filename, strerror(errno));
		return -1;
	}

	fprintf(f, ".equ %s_pal_len %d\n", name, m_iPalCount*8);
	fprintf(f, ".section \"s_%s_pal\" align 16\n", name);
	fprintf(f, "%s_pal:\n", name);

	for (u32 i=0;i<m_iPalCount;i++)
	{
		fprintf(f, ".DW $%04x, $%04x, $%04x, $%04x\n", m_pPalData[(i*4)+0], m_pPalData[(i*4)+1], m_pPalData[(i*4)+2], m_pPalData[(i*4)+3]);
	}
	fprintf(f, "%s_pal_end:\n", name);
	fprintf(f, ".ends\n");
	fclose(f);
	return 0;
}

int GbcConv::writeChr(const char *filename, const char *name)
{
	dinfo("%s: writing characters", filename);
	FILE *f = fopen(filename, "w");
	if (f==NULL)
	{
		dwarn("%s: failed to open (%s)", filename, strerror(errno));
		return -1;
	}

	fprintf(f, ".equ %s_chr_len %d\n", name, m_iChrCount*16);
	fprintf(f, ".section \"s_%s_chr\" align 16\n", name);
	fprintf(f, "%s_chr:\n", name);

	for (u32 i=0;i<m_iChrCount;i++)
	{
		u16 *chrWord = (u16*)&m_pChrData[(i*16)];
		fprintf(f, ".DW $%04x, $%04x, $%04x, $%04x, $%04x, $%04x, $%04x, $%04x\n", chrWord[0], chrWord[1], chrWord[2], chrWord[3], chrWord[4], chrWord[5], chrWord[6], chrWord[7]);
	}
	fprintf(f, "%s_chr_end:\n", name);

	fprintf(f, ".ends\n");
	fclose(f);
	return 0;
}

int GbcConv::writeMap(const char *filename, const char *name)
{
	dinfo("%s: writing map...", filename);
	FILE *f = fopen(filename, "w");
	if (f==NULL)
	{
		dwarn("%s: failed to open (%s)", filename, strerror(errno));
		return -1;
	}

	// both h/l are the same length . but having them as well might helps macros...
	fprintf(f, ".equ %s_map_len %d\n", name, m_iBlkCount);
	fprintf(f, ".equ %s_mapl_len %d\n", name, m_iBlkCount);
	fprintf(f, ".equ %s_maph_len %d\n", name, m_iBlkCount);

	fprintf(f, ".section \"s_%s_mapl\" align 16\n", name);
	fprintf(f, "%s_mapl:\n", name);
	for (u32 i=0;i<m_iBlkCount;i++)
	{
		fprintf(f, "  .DB $%02x\n", m_pBlkChr[i]&0xff);
	}
	fprintf(f, ".ends\n");

	fprintf(f, ".section \"s_%s_maph\" align 16\n", name);
	fprintf(f, "%s_maph:\n", name);
	for (u32 i=0;i<m_iBlkCount;i++)
	{
		fprintf(f, "  .DB $%02x\n", m_pBlkPal[i]&7);
	}
	fprintf(f, ".ends\n");

	fclose(f);
	return 0;
}

int GbcConv::readPal(const char *filename)
{
	m_iPalCount = 0;
	dinfo("%s: reading palettes", filename);
	FILE *f = fopen(filename, "r");
	if (f==NULL)
	{
		dwarn("%s: failed to open (%s)", filename, strerror(errno));
		return -1;
	}
	m_iPalCount = 0;
	while (!feof(f))
	{
		char buf[512];
			
		if (fgets(buf, sizeof(buf), f)==NULL)
			break;
		unsigned int localPal[4] = {0,0,0,0};
		if (sscanf(buf, ".DW $%04x, $%04x, $%04x, $%04x", &localPal[0], &localPal[1], &localPal[2], &localPal[3])<4)
			continue;
		printf("pal%02x = %04x %04x %04x %04x\n", m_iPalCount, localPal[0], localPal[1], localPal[2], localPal[3]);
		m_pPalData[(m_iPalCount*4)+0] = localPal[0];
		m_pPalData[(m_iPalCount*4)+1] = localPal[1];
		m_pPalData[(m_iPalCount*4)+2] = localPal[2];
		m_pPalData[(m_iPalCount*4)+3] = localPal[3];
		m_iPalCount++;
	}
	fclose(f);

	return 0;
}
int GbcConv::imageDoChr()
{
	for (u32 chr=0;chr<m_iBlkCount;chr++)
	{
		u8 chrLocal[16] = {0};

		int blkx = (chr%(m_pSurface->info.width/8))*8;
		int blky = (chr/(m_pSurface->info.width/8))*8;
		u8 *datL = (u8*)m_pSurface->buffer + (blky*m_pSurface->info.pitch) + (blkx*2);

		if (m_pBlkPal[chr]<0)
			continue;


		for (uint32_t y=0;y<8;y++)
		{
			u16 *dat = (u16*)datL;
			for (uint32_t x=0;x<8;x++)
			{
				int color = 0;
				for (int i=0;i<4;i++)
				{
					if (*dat==m_pPalData[(m_pBlkPal[chr]*4)+i])
					{
						color = i;
						break;
					}
				}

				chrLocal[(y*2)+0] |=  (color&1)    <<(7-x);
				chrLocal[(y*2)+1] |= ((color&2)>>1)<<(7-x);

				dat++;
			}
			datL += m_pSurface->info.pitch;
		}

		int foundChr = findChr(chrLocal);
		if (foundChr >= 0)
		{
			m_pBlkChr[chr] = foundChr;
			continue;
		}
		memcpy(&m_pChrData[(m_iChrCount*16)], chrLocal, 16);
		m_pBlkChr[chr] = m_iChrCount++;
	}
	return 0;
}

int GbcConv::imageDoPal(bool generate)
{
	if (generate) m_iPalCount = 0;

	for (unsigned int chr=0;chr<m_iBlkCount;chr++)
	{
		int blkx = (chr%(m_pSurface->info.width/8))*8;
		int blky = (chr/(m_pSurface->info.width/8))*8;
		u8 *datL = (u8*)m_pSurface->buffer + (blky*m_pSurface->info.pitch) + (blkx*2);

		u16 localPal[4] = {0x8000, 0x8000, 0x8000, 0x8000}; // no colours initially
		int localCur = 0;

		for (uint32_t y=0;y<8;y++)
		{
			u16 *dat = (u16*)datL;
			for (uint32_t x=0;x<8;x++)
			{
				if (*dat!=localPal[0] && *dat!=localPal[1] && *dat!=localPal[2] && *dat!=localPal[3])
				{
					if (localCur<4)
						localPal[localCur] = *dat;
					localCur++;
				}
				dat++;
			}
			if (localCur>4)
				break;

			datL += m_pSurface->info.pitch;
		}
		if (localCur>4)
		{
			m_pBlkErr[chr] |= ERROR_MANYCOLORS;
			printf("block at %dx%d has too many colours!\n", blkx, blky);
		}
		int palIdx = findPal(localPal, generate);
		if (palIdx>=0)
		{
			m_pBlkPal[chr]=palIdx;
			continue;
		}

		if (!generate)
		{
			printf("block at %dx%d has no matching palette (%04x %04x %04x %04x)!\n", blkx, blky, localPal[0], localPal[1], localPal[2], localPal[3]);
			m_pBlkPal[chr]=-1;
			m_pBlkErr[chr] |= ERROR_NOPAL;
			continue;
		}


		printf("pal%02x = %04x %04x %04x %04x\n", m_iPalCount, localPal[0], localPal[1], localPal[2], localPal[3]);

		m_pPalData[(m_iPalCount*4)+0] = localPal[0];
		m_pPalData[(m_iPalCount*4)+1] = localPal[1];
		m_pPalData[(m_iPalCount*4)+2] = localPal[2];
		m_pPalData[(m_iPalCount*4)+3] = localPal[3];
		m_pBlkPal[chr]=m_iPalCount++;
	}

	return 0;
}


GbcConv::GbcConv(RasterSurface *surface)
{
	m_pSurface  = surface;
	m_iBlkCount = (m_pSurface->info.width/8) * (m_pSurface->info.height/8);

	m_pChrData = new u8[m_iBlkCount*16];
	m_pPalData = new u16[m_iBlkCount*4];

	m_pBlkPal = new int[m_iBlkCount];
	m_pBlkChr = new int[m_iBlkCount];
	m_pBlkErr = new int[m_iBlkCount];

	m_iChrCount = 0;
	m_iPalCount = 0;

	imageResetErrors();
}

int GbcConv::findChr(u8 *thisChr)
{
	for (unsigned int i=0;i<m_iChrCount;i++)
	{
		int match=0;
		for (match=0;match<16;match++)
		{
			if (m_pChrData[(i*16)+match]!=thisChr[match])
				break;
		}
		
		if (match==16)
			return i;
	}

	return -1;
}

int GbcConv::findPal(u16 *colours, bool append)
{

	for (unsigned int pal=0;pal<m_iPalCount;pal++)
	{
		int match = 0;
		int ignore = 0;
		int empty = 0;
		int emptyPal = 0;
		for (int i=0;i<4;i++)
		{
			if (colours[i]&0x8000)
			{
				empty++;
				ignore |= 1<<i;
				continue;
			}
			for (int j=0;j<4;j++)
			{
				if (m_pPalData[(pal*4)+j]&0x8000)
				{
					emptyPal++;
					continue;	
				}

				if (m_pPalData[(pal*4)+j]==colours[i])
				{
	                match++;
					ignore |= 1<<i;
					break;
				}
			}
		}

		if ((match+empty) == 4)
			return pal;

		if (!append)
			continue;

		if (emptyPal<(4-match-empty))
			continue;

		for (int i=0;i<4;i++)
		{
			if (ignore&(1<<i))
				continue;

			for (int j=0;j<4;j++)
			{
				if (m_pPalData[(pal*4)+j]&0x8000)
				{
					m_pPalData[(pal*4)+j] = colours[i];
					break;
				}
			}
		}
		printf("pal%02x~= %04x %04x %04x %04x\n", pal, m_pPalData[(pal*4)+0], m_pPalData[(pal*4)+1], m_pPalData[(pal*4)+2], m_pPalData[(pal*4)+3]);
		return pal;
	}

	return -1;
}



// bringing in raster kit causes issues with QT.. find out why... but until then...... h4x0r0r0r0r

void rasterCopyClut(RasterSurface *target, RasterSurface *source, RasterSurface *sClut)
{
	RasterRect rect;

	rect.x = 0;
	rect.y = 0;
	rect.w = target->info.width;
	rect.h = target->info.width;
	if (source->info.width<rect.w)
		rect.w = source->info.width;

	if (source->info.height<rect.h)
		rect.h = source->info.height;

	rasterBltClut(target, &rect, source, &rect, sClut);
}


void rasterBltClut(RasterSurface *target, RasterRect *tRect, RasterSurface *source, RasterRect *sRect, RasterSurface *)
{
	assert(target->info.bpp==2);
	assert(source->info.bpp==4);

	u8 *srcL = (u8*)source->buffer;
	u8 *tgtL = (u8*)target->buffer;

	srcL += (tRect->y*target->info.pitch) + (tRect->x*target->info.bpp);
	tgtL += (sRect->y*source->info.pitch) + (sRect->x*source->info.bpp);

	for (uint32_t y=0;y<tRect->h;y++)
	{
		u8 *src = srcL, *tgt=tgtL;
		for (uint32_t x=0;x<tRect->w;x++)
		{
			u8 ch[4];
			ch[0] = src[0];
			ch[1] = src[1];
			ch[2] = src[2];
			ch[3] = src[3];
			ch[0] >>= 3;
			ch[1] >>= 3;
			ch[2] >>= 3;
			*(u16*)tgt = ch[0] | ch[1] << 5 | ch[2] << 10;
			src+=source->info.bpp;
			tgt+=target->info.bpp;
		}
		srcL += source->info.pitch;
		tgtL += target->info.pitch;
	}
}

extern "C" {const char *debugGetLine(int id, int *level);}

void shoveError(const char *name, QWidget *widget = NULL)
{

	int level;
	QString str;

	const char *foo="";
	for (int i=0;(foo=debugGetLine(i, &level));i++)
	{
		str = QString(foo) + QString("\n") + str;
	}
	QMessageBox::warning(widget, name, str);
}





void EditorShow::mouseMoveEvent  ( QMouseEvent * event )  {hax->mouse(event->x(), event->y()); }
void EditorShow::mousePressEvent ( QMouseEvent * event )  {hax->mouse(event->x(), event->y()); }

void EditorWindow::mouse(int x, int y)
{
	int blk;
	if (x<0 || y<0 || x>=(width*4) || y>=(height*4))
	{
		blk = -1;
	}
	else
	{
		x/=4; // to pixels
		y/=4;

		x/=8; // to blocks
		y/=8;

		blk = x + (y*((width)/8));

		if (blk<0 || blk>=(int)conv->m_iBlkCount)
			blk = -1;
	}

	if (blk!=curBlk)
	{
		curBlk = blk;
		curBlkX = x;
		curBlkY = y;
		drawStuff();
	}
	

}

void EditorWindow::reloadConv()
{
	if (conv!=NULL)
	{
		delete conv;	
		conv = NULL;
	}

	OptionList *loadOptions = (OptionList *)alloca(sizeof(OptionList)+(sizeof(OptionListSlot)*4));
	optionInit(loadOptions, 4);
	optionAddValue(loadOptions, "AlwaysRGBA8888", "yes");
	Image *image = imageLoad(origFilename, loadOptions);
	optionReset(loadOptions);
	if (image==NULL)
	{
		shoveError("Can't open image", this);
		exit(1);
	}

	if (surface.buffer!=NULL)
		free(surface.buffer);

	// convert image into a single 16bpp surface
	surface.info.width  = image->surfaces[0].raster.info.width;
	surface.info.height = image->surfaces[0].raster.info.height;
	surface.info.format = RASTER_FORMAT_RGBA5551;
	surface.info.bpp    = 2;
	surface.info.pitch  = surface.info.width*surface.info.bpp;
	surface.buffer      = malloc(surface.info.pitch*surface.info.height);
	rasterCopyClut(&surface, &image->surfaces[0].raster, &image->surfaces[0].clut);
	imageDestroy(image);


	conv = new GbcConv(&surface);
	conv->imageDoPal(true);
	conv->imageDoChr();
}


void EditorWindow::loadPalFunc()
{
	if (conv==NULL)
		return;
	QString filenameString = QFileDialog::getOpenFileName(NULL, "Open Palette", NULL, "ASM Palette files (*.pal.s)");

	if (filenameString.isNull())
	{
		return;
	}
	QByteArray filenameBytes = filenameString.toUtf8();
	const char *filename = filenameBytes.data();

	if (conv->readPal(filename)<0)
	{
		shoveError("Reading palette", this);
		return;
	}

	conv->imageDoPal(false);
	conv->imageDoChr();

}

void EditorWindow::saveStuff()
{
	if (conv==NULL)
		return;

	QString nameString = namebox->text();
	QByteArray nameBytes = nameString.toUtf8();
	const char *name = nameBytes.data();

	if (name[0]=='\0')
	{
		QMessageBox::warning(NULL, "Invalid name", "the name is empty");
		return;
	}

	for (u32 i=0;i<strlen(name);i++)
	{
		if (!((name[i]>='a'&&name[i]<='z') || (name[i]>='A'&&name[i]<='Z') || (name[i]>='0'&&name[i]<='9') || (name[i]=='_')))
		{
			QMessageBox::warning(NULL, "Invalid name", "there are bad characters in the name");
			return;
		}
	}

	char buf[512];
	QString origdir;
	if (getpath(buf, sizeof(buf), origFilename)>=0)
		origdir = QString(buf);

	QString dirString = QFileDialog::getExistingDirectory(this, "Select directory to save to...", origdir);
	if (dirString.isNull())
		return;
	QByteArray dirBytes = dirString.toUtf8();
	const char *dir = dirBytes.data();



	snprintf(buf, sizeof(buf), "%s/%s.pal.s", dir, name);
	if (conv->writePal(buf, name)<0)
		shoveError("Writing palette data..", this);

	snprintf(buf, sizeof(buf), "%s/%s.chr.s", dir, name);
	if (conv->writeChr(buf, name)<0)
		shoveError("Writing character data..", this);

	snprintf(buf, sizeof(buf), "%s/%s.map.s", dir, name);
	if (conv->writeMap(buf, name)<0)
		shoveError("Writing map data..", this);

}


EditorWindow::EditorWindow(const char *afilename)
{
	setWindowTitle("Editor");
	
	surface.buffer = NULL;
	conv = NULL;
	m_bShowErrors = true;

	origFilename = afilename;
	reloadConv();

	width = conv->m_pSurface->info.width;
	height = conv->m_pSurface->info.height;
	curBlk = 0; curBlkX=0; curBlkY=0;

	QImage fooImage  = QImage((uchar*)conv->m_pSurface->buffer, width, height, conv->m_pSurface->info.pitch, QImage::Format_RGB555);
	ourImage = fooImage.rgbSwapped();
	resultImage = QImage(width*4, height*4, QImage::Format_ARGB32_Premultiplied);

	blockInfo = new QLabel;
    blockInfo->setMinimumWidth(120);
	blockInfo->setFont(QFont("Monospace"));

    resultLabel = new EditorShow(this);
    resultLabel->setMinimumWidth(width*4);
    resultLabel->setMinimumHeight(height*4);
	drawStuff();

	char buf[32]; 
	int bufSize = getfilename(buf, sizeof(buf), origFilename);
	if (bufSize<0)
		namebox = new QLineEdit("blah");
	else
	{
		for (u32 i=0;i<strlen(buf);i++)
		{
			if (!((buf[i]>='a'&&buf[i]<='z') || (buf[i]>='A'&&buf[i]<='Z') || (buf[i]>='0'&&buf[i]<='9') || (buf[i]=='_')))
				buf[i] = '_';
		}
		namebox = new QLineEdit(buf);
	}
	reload = new QPushButton("Reload");
	loadPal = new QPushButton("Load Palette");
	saveSet = new QPushButton("Save");

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(resultLabel, 0, 0, 10, 10);
    mainLayout->addWidget(blockInfo,   10, 0, 1, 10);


    mainLayout->addWidget(new QLabel("Name:"), 0, 10, 1, 1);
    mainLayout->addWidget(namebox, 1, 10, 1, 1);
    mainLayout->addWidget(reload,      3, 10, 1, 1);
    mainLayout->addWidget(loadPal,     4, 10, 1, 1);
    mainLayout->addWidget(saveSet,     5, 10, 1, 1);

    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    setLayout(mainLayout);

    timer = new QTimer(this);
    connect(timer,  SIGNAL(timeout()), this, SLOT(timerTick()));
    connect(reload, SIGNAL(clicked()), this, SLOT(reloadConv()));
    connect(saveSet, SIGNAL(clicked()), this, SLOT(saveStuff()));
    connect(loadPal, SIGNAL(clicked()), this, SLOT(loadPalFunc()));
    timer->start(500);
}

void EditorWindow::timerTick()
{
	m_bShowErrors = !m_bShowErrors;

	if (conv!=NULL)
		drawStuff();
}

void EditorWindow::drawStuff()
{
	if (!conv)
		return;

    QPainter painter(&resultImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(resultImage.rect(), Qt::black);
    painter.drawImage(QRectF(0, 0, (width*4), (height*4)), ourImage, QRectF(0, 0, width, height));

	if (m_bShowErrors)
	{
		for (u32 chr=0;chr<conv->m_iBlkCount;chr++)
		{
			if (conv->m_pBlkErr[chr])
			{
				int blkx = (chr%(conv->m_pSurface->info.width/8))*8;
				int blky = (chr/(conv->m_pSurface->info.width/8))*8;
		    	painter.fillRect(QRectF(blkx*4, blky*4, 8*4, 8*4), QColor(255,0,0));
			}
		}
	}


	if (curBlk>=0)
	{
		painter.setPen (QColor(0,255,0));
    	painter.drawLine((curBlkX*32)+0 , (curBlkY*32)+0 , (curBlkX*32)+32, (curBlkY*32)+0 );
    	painter.drawLine((curBlkX*32)+0 , (curBlkY*32)+32, (curBlkX*32)+32, (curBlkY*32)+32);
    	painter.drawLine((curBlkX*32)+0 , (curBlkY*32)+0,  (curBlkX*32)+0 , (curBlkY*32)+32);
    	painter.drawLine((curBlkX*32)+32, (curBlkY*32)+0,  (curBlkX*32)+32, (curBlkY*32)+32);
		QString info;
		info.sprintf("Block: %d (@%d,%d)\nchr=%d, pal=%d\n", curBlk, curBlkX*8, curBlkY*8, conv->m_pBlkChr[curBlk], conv->m_pBlkPal[curBlk]);

		if (conv->m_pBlkErr[curBlk]&ERROR_NOPAL)      info += "No palette! ";
		if (conv->m_pBlkErr[curBlk]&ERROR_MANYCOLORS) info += "Too many colours!";

		info += "\n";

		for (int y=0;y<8;y++)
		{
			u8 *foo = &conv->m_pChrData[(conv->m_pBlkChr[curBlk]*16)+(y*2)];

			for (int x=7;x>=0;x--)
			{
				QString line;
				int id = 0;
				if (foo[0]&(1<<x)) id+=1;
				if (foo[1]&(1<<x)) id+=2;
				line.sprintf("%d ", id);
				info+=line;
			}
			info += QString("\n");
		}

		blockInfo->setText(info);
	}
	else
	{
		blockInfo->setText("No block!");
	}


/*
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage(0, 0, destinationImage);
    painter.setCompositionMode(mode);
    painter.drawImage(0, 0, sourceImage);
    painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    painter.fillRect(resultImage.rect(), Qt::white);
*/
    painter.end();

    resultLabel->setPixmap(QPixmap::fromImage(resultImage));
}


int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	
	QString filenameString;

	if (argc<2)
	{
		filenameString = QFileDialog::getOpenFileName(NULL, "Open Image", NULL, "Image Files (*.png)");
	}
	else
	{
		filenameString = QString(argv[1]);
	}

	if (filenameString.isNull())
	{
		return 1;
	}
	QByteArray filenameBytes = filenameString.toUtf8();
	const char *filename = filenameBytes.data();

	printf("filename: %s\n", filename);
	
	EditorWindow *window = new EditorWindow(filename);
	window->show();
	return app.exec();
}



