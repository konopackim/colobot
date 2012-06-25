// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.

// modfile.cpp


#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "common/struct.h"
#include "math/geometry.h"
#include "old/d3dengine.h"
#include "old/d3dmath.h"
#include "common/language.h"
#include "common/event.h"
#include "common/misc.h"
#include "common/iman.h"
#include "old/math3d.h"
#include "modfile.h"



const int MAX_VERTICES = 2000;



// Object's constructor.

CModFile::CModFile(CInstanceManager* iMan)
{
	m_iMan = iMan;

	m_engine = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);

	m_triangleUsed = 0;
	m_triangleTable = (ModelTriangle*)malloc(sizeof(ModelTriangle)*MAX_VERTICES);
	ZeroMemory(m_triangleTable, sizeof(ModelTriangle)*MAX_VERTICES);
}

// Object's destructor.

CModFile::~CModFile()
{
	free(m_triangleTable);
}




// Creates a triangle in the internal structure.

bool CModFile::CreateTriangle(Math::Vector p1, Math::Vector p2, Math::Vector p3,
							  float min, float max)
{
	Math::Vector	n;
	int			i;

	if ( m_triangleUsed >= MAX_VERTICES )
	{
		OutputDebugString("ERROR: CreateTriangle::Too many triangles\n");
		return false;
	}

	i = m_triangleUsed++;

	ZeroMemory(&m_triangleTable[i], sizeof(ModelTriangle));

	m_triangleTable[i].bUsed = true;
	m_triangleTable[i].bSelect = false;

	n = Math::NormalToPlane(p3, p2, p1);
	m_triangleTable[i].p1 = D3DVERTEX2( p1, n);
	m_triangleTable[i].p2 = D3DVERTEX2( p2, n);
	m_triangleTable[i].p3 = D3DVERTEX2( p3, n);

	m_triangleTable[i].material.diffuse.r = 1.0f;
	m_triangleTable[i].material.diffuse.g = 1.0f;
	m_triangleTable[i].material.diffuse.b = 1.0f;  // white
	m_triangleTable[i].material.ambient.r = 0.5f;
	m_triangleTable[i].material.ambient.g = 0.5f;
	m_triangleTable[i].material.ambient.b = 0.5f;

	m_triangleTable[i].min = min;
	m_triangleTable[i].max = max;

	return true;
}

// Reads a DXF file.

bool CModFile::ReadDXF(char *filename, float min, float max)
{
	FILE*		file = NULL;
	char		line[100];
	int			command, rankSommet, nbSommet, nbFace;
	Math::Vector	table[MAX_VERTICES];
	bool		bWaitNbSommet;
	bool		bWaitNbFace;
	bool		bWaitSommetX;
	bool		bWaitSommetY;
	bool		bWaitSommetZ;
	bool		bWaitFaceX;
	bool		bWaitFaceY;
	bool		bWaitFaceZ;
	float		x,y,z;
	int			p1,p2,p3;

	file = fopen(filename, "r");
	if ( file == NULL )  return false;

	m_triangleUsed = 0;

	rankSommet = 0;
	bWaitNbSommet = false;
	bWaitNbFace   = false;
	bWaitSommetX  = false;
	bWaitSommetY  = false;
	bWaitSommetZ  = false;
	bWaitFaceX    = false;
	bWaitFaceY    = false;
	bWaitFaceZ    = false;

	while ( fgets(line, 100, file) != NULL )
	{
		sscanf(line, "%d", &command);
		if ( fgets(line, 100, file) == NULL )  break;

		if ( command == 66 )
		{
			bWaitNbSommet = true;
		}

		if ( command == 71 && bWaitNbSommet )
		{
			bWaitNbSommet = false;
			sscanf(line, "%d", &nbSommet);
			if ( nbSommet > MAX_VERTICES )  nbSommet = MAX_VERTICES;
			rankSommet = 0;
			bWaitNbFace = true;

//?			sprintf(s, "Waiting for %d sommets\n", nbSommet);
//?			OutputDebugString(s);
		}

		if ( command == 72 && bWaitNbFace )
		{
			bWaitNbFace = false;
			sscanf(line, "%d", &nbFace);
			bWaitSommetX = true;

//?			sprintf(s, "Waiting for %d faces\n", nbFace);
//?			OutputDebugString(s);
		}

		if ( command == 10 && bWaitSommetX )
		{
			bWaitSommetX = false;
			sscanf(line, "%f", &x);
			bWaitSommetY = true;
		}

		if ( command == 20 && bWaitSommetY )
		{
			bWaitSommetY = false;
			sscanf(line, "%f", &y);
			bWaitSommetZ = true;
		}

		if ( command == 30 && bWaitSommetZ )
		{
			bWaitSommetZ = false;
			sscanf(line, "%f", &z);

			nbSommet --;
			if ( nbSommet >= 0 )
			{
				Math::Vector p(x,z,y);  // permutation of Y and Z!
				table[rankSommet++] = p;
				bWaitSommetX = true;

//?				sprintf(s, "Sommet[%d]=%f;%f;%f\n", rankSommet, p.x,p.y,p.z);
//?				OutputDebugString(s);
			}
			else
			{
				bWaitFaceX = true;
			}
		}

		if ( command == 71 && bWaitFaceX )
		{
			bWaitFaceX = false;
			sscanf(line, "%d", &p1);
			if ( p1 < 0 )  p1 = -p1;
			bWaitFaceY = true;
		}

		if ( command == 72 && bWaitFaceY )
		{
			bWaitFaceY = false;
			sscanf(line, "%d", &p2);
			if ( p2 < 0 )  p2 = -p2;
			bWaitFaceZ = true;
		}

		if ( command == 73 && bWaitFaceZ )
		{
			bWaitFaceZ = false;
			sscanf(line, "%d", &p3);
			if ( p3 < 0 )  p3 = -p3;

			nbFace --;
			if ( nbFace >= 0 )
			{
				CreateTriangle( table[p3-1], table[p2-1], table[p1-1], min,max );
				bWaitFaceX = true;

//?				sprintf(s, "Face=%d;%d;%d\n", p1,p2,p3);
//?				OutputDebugString(s);
			}
		}

	}

	fclose(file);
	return true;
}



struct InfoMOD
{
	int		rev;
	int		vers;
	int		total;
	int		reserve[10];
};


// Change nom.bmp to nom.tga

void ChangeBMPtoTGA(char *filename)
{
	char*	p;

	p = strstr(filename, ".bmp");
	if ( p != 0 )  strcpy(p, ".tga");
}


// Reads a MOD file.

bool CModFile::AddModel(char *filename, int first, bool bEdit, bool bMeta)
{
	FILE*		file;
	InfoMOD		info;
	float		limit[2];
	int			i, nb, err;
	char*		p;

	if ( m_engine->RetDebugMode() )
	{
		bMeta = false;
	}

	if ( bMeta )
	{
		p = strchr(filename, '\\');
		if ( p == 0 )
		{
#if _SCHOOL
			err = g_metafile.Open("ceebot2.dat", filename);
#else
			err = g_metafile.Open("colobot2.dat", filename);
#endif
		}
		else
		{
#if _SCHOOL
			err = g_metafile.Open("ceebot2.dat", p+1);
#else
			err = g_metafile.Open("colobot2.dat", p+1);
#endif
		}
		if ( err != 0 )  bMeta = false;
	}
	if ( !bMeta )
	{
		file = fopen(filename, "rb");
		if ( file == NULL )  return false;
	}

	if ( bMeta )
	{
		g_metafile.Read(&info, sizeof(InfoMOD));
	}
	else
	{
		fread(&info, sizeof(InfoMOD), 1, file);
	}
	nb = info.total;
	m_triangleUsed += nb;

	if ( info.rev == 1 && info.vers == 0 )
	{
		OldModelTriangle1	old;

		for ( i=first ; i<m_triangleUsed ; i++ )
		{
			if ( bMeta )
			{
				g_metafile.Read(&old, sizeof(OldModelTriangle1));
			}
			else
			{
				fread(&old, sizeof(OldModelTriangle1), 1, file);
			}

			ZeroMemory(&m_triangleTable[i], sizeof(ModelTriangle));
			m_triangleTable[i].bUsed = old.bUsed;
			m_triangleTable[i].bSelect = old.bSelect;

			m_triangleTable[i].p1.x = old.p1.x;
			m_triangleTable[i].p1.y = old.p1.y;
			m_triangleTable[i].p1.z = old.p1.z;
			m_triangleTable[i].p1.nx = old.p1.nx;
			m_triangleTable[i].p1.ny = old.p1.ny;
			m_triangleTable[i].p1.nz = old.p1.nz;
			m_triangleTable[i].p1.tu = old.p1.tu;
			m_triangleTable[i].p1.tv = old.p1.tv;

			m_triangleTable[i].p2.x = old.p2.x;
			m_triangleTable[i].p2.y = old.p2.y;
			m_triangleTable[i].p2.z = old.p2.z;
			m_triangleTable[i].p2.nx = old.p2.nx;
			m_triangleTable[i].p2.ny = old.p2.ny;
			m_triangleTable[i].p2.nz = old.p2.nz;
			m_triangleTable[i].p2.tu = old.p2.tu;
			m_triangleTable[i].p2.tv = old.p2.tv;

			m_triangleTable[i].p3.x = old.p3.x;
			m_triangleTable[i].p3.y = old.p3.y;
			m_triangleTable[i].p3.z = old.p3.z;
			m_triangleTable[i].p3.nx = old.p3.nx;
			m_triangleTable[i].p3.ny = old.p3.ny;
			m_triangleTable[i].p3.nz = old.p3.nz;
			m_triangleTable[i].p3.tu = old.p3.tu;
			m_triangleTable[i].p3.tv = old.p3.tv;

			m_triangleTable[i].material = old.material;
			strcpy(m_triangleTable[i].texName, old.texName);
			m_triangleTable[i].min = old.min;
			m_triangleTable[i].max = old.max;
		}
	}
	else if ( info.rev == 1 && info.vers == 1 )
	{
		OldModelTriangle2	old;

		for ( i=first ; i<m_triangleUsed ; i++ )
		{
			if ( bMeta )
			{
				g_metafile.Read(&old, sizeof(OldModelTriangle2));
			}
			else
			{
				fread(&old, sizeof(OldModelTriangle2), 1, file);
			}

			ZeroMemory(&m_triangleTable[i], sizeof(ModelTriangle));
			m_triangleTable[i].bUsed = old.bUsed;
			m_triangleTable[i].bSelect = old.bSelect;

			m_triangleTable[i].p1.x = old.p1.x;
			m_triangleTable[i].p1.y = old.p1.y;
			m_triangleTable[i].p1.z = old.p1.z;
			m_triangleTable[i].p1.nx = old.p1.nx;
			m_triangleTable[i].p1.ny = old.p1.ny;
			m_triangleTable[i].p1.nz = old.p1.nz;
			m_triangleTable[i].p1.tu = old.p1.tu;
			m_triangleTable[i].p1.tv = old.p1.tv;

			m_triangleTable[i].p2.x = old.p2.x;
			m_triangleTable[i].p2.y = old.p2.y;
			m_triangleTable[i].p2.z = old.p2.z;
			m_triangleTable[i].p2.nx = old.p2.nx;
			m_triangleTable[i].p2.ny = old.p2.ny;
			m_triangleTable[i].p2.nz = old.p2.nz;
			m_triangleTable[i].p2.tu = old.p2.tu;
			m_triangleTable[i].p2.tv = old.p2.tv;

			m_triangleTable[i].p3.x = old.p3.x;
			m_triangleTable[i].p3.y = old.p3.y;
			m_triangleTable[i].p3.z = old.p3.z;
			m_triangleTable[i].p3.nx = old.p3.nx;
			m_triangleTable[i].p3.ny = old.p3.ny;
			m_triangleTable[i].p3.nz = old.p3.nz;
			m_triangleTable[i].p3.tu = old.p3.tu;
			m_triangleTable[i].p3.tv = old.p3.tv;

			m_triangleTable[i].material = old.material;
			strcpy(m_triangleTable[i].texName, old.texName);
			m_triangleTable[i].min = old.min;
			m_triangleTable[i].max = old.max;
			m_triangleTable[i].state = old.state;
			m_triangleTable[i].reserve2 = old.reserve2;
			m_triangleTable[i].reserve3 = old.reserve3;
			m_triangleTable[i].reserve4 = old.reserve4;
		}
	}
	else
	{
		if ( bMeta )
		{
			g_metafile.Read(m_triangleTable+first, sizeof(ModelTriangle)*nb);
		}
		else
		{
			fread(m_triangleTable+first, sizeof(ModelTriangle), nb, file);
		}
	}

	for ( i=first ; i<m_triangleUsed ; i++ )
	{
		ChangeBMPtoTGA(m_triangleTable[i].texName);
	}

	if ( !bEdit )
	{
		limit[0] = m_engine->RetLimitLOD(0);  // frontier AB as config
		limit[1] = m_engine->RetLimitLOD(1);  // frontier BC as config

		// Standard frontiers -> config.
		for ( i=first ; i<m_triangleUsed ; i++ )
		{
			if ( m_triangleTable[i].min == 0.0f &&
				 m_triangleTable[i].max == 100.0f )  // resolution A ?
			{
				m_triangleTable[i].max = limit[0];
			}
			else if ( m_triangleTable[i].min == 100.0f &&
					  m_triangleTable[i].max == 200.0f )  // resolution B ?
			{
				m_triangleTable[i].min = limit[0];
				m_triangleTable[i].max = limit[1];
			}
			else if ( m_triangleTable[i].min == 200.0f &&
					  m_triangleTable[i].max == 1000000.0f )  // resolution C ?
			{
				m_triangleTable[i].min = limit[1];
			}
		}
	}

	if ( bMeta )
	{
		g_metafile.Close();
	}
	else
	{
		fclose(file);
	}
	return true;
}

// Reads a MOD file.

bool CModFile::ReadModel(char *filename, bool bEdit, bool bMeta)
{
	m_triangleUsed = 0;
	return AddModel(filename, 0, bEdit, bMeta);
}


// Writes a MOD file.

bool CModFile::WriteModel(char *filename)
{
	FILE*		file;
	InfoMOD		info;

	if ( m_triangleUsed == 0 )  return false;

	file = fopen(filename, "wb");
	if ( file == NULL )  return false;

	ZeroMemory(&info, sizeof(InfoMOD));
	info.rev   = 1;
	info.vers  = 2;
	info.total = m_triangleUsed;
	fwrite(&info, sizeof(InfoMOD), 1, file);

	fwrite(m_triangleTable, sizeof(ModelTriangle), m_triangleUsed, file);

	fclose(file);
	return true;
}


// Creates the object in the 3D engine.

bool CModFile::CreateEngineObject(int objRank, int addState)
{
#if 0
	char	texName2[20];
	int		texNum, i, state;

	for ( i=0 ; i<m_triangleUsed ; i++ )
	{
		if ( !m_triangleTable[i].bUsed )  continue;

		state = m_triangleTable[i].state;
		texName2[0] = 0;

		if ( m_triangleTable[i].texNum2 != 0 )
		{
			if ( m_triangleTable[i].texNum2 == 1 )
			{
				texNum = m_engine->RetSecondTexture();
			}
			else
			{
				texNum = m_triangleTable[i].texNum2;
			}

			if ( texNum >= 1 && texNum <= 10 )
			{
				state = m_triangleTable[i].state|D3DSTATEDUALb;
			}
			if ( texNum >= 11 && texNum <= 20 )
			{
				state = m_triangleTable[i].state|D3DSTATEDUALw;
			}
			sprintf(texName2, "dirty%.2d.bmp", texNum);
		}

		m_engine->AddTriangle(objRank, &m_triangleTable[i].p1, 3,
							  m_triangleTable[i].material,
							  state+addState,
							  m_triangleTable[i].texName, texName2,
							  m_triangleTable[i].min,
							  m_triangleTable[i].max, false);
	}
	return true;
#else
	char	texName1[20];
	char	texName2[20];
	int		texNum, i, state;

	for ( i=0 ; i<m_triangleUsed ; i++ )
	{
		if ( !m_triangleTable[i].bUsed )  continue;

		state = m_triangleTable[i].state;
		strcpy(texName1, m_triangleTable[i].texName);
		texName2[0] = 0;

		if ( strcmp(texName1, "plant.tga") == 0 )
		{
			state |= D3DSTATEALPHA;
		}

		if ( m_triangleTable[i].texNum2 != 0 )
		{
			if ( m_triangleTable[i].texNum2 == 1 )
			{
				texNum = m_engine->RetSecondTexture();
			}
			else
			{
				texNum = m_triangleTable[i].texNum2;
			}

			if ( texNum >= 1 && texNum <= 10 )
			{
				state |= D3DSTATEDUALb;
			}
			if ( texNum >= 11 && texNum <= 20 )
			{
				state |= D3DSTATEDUALw;
			}
			sprintf(texName2, "dirty%.2d.tga", texNum);
		}

		m_engine->AddTriangle(objRank, &m_triangleTable[i].p1, 3,
							  m_triangleTable[i].material,
							  state+addState,
							  texName1, texName2,
							  m_triangleTable[i].min,
							  m_triangleTable[i].max, false);
	}
	return true;
#endif
}


// Performs a mirror according to Z.

void CModFile::Mirror()
{
	D3DVERTEX2	t;
	int			i;

	for ( i=0 ; i<m_triangleUsed ; i++ )
	{
		t                     = m_triangleTable[i].p1;
		m_triangleTable[i].p1 = m_triangleTable[i].p2;
		m_triangleTable[i].p2 = t;

		m_triangleTable[i].p1.z = -m_triangleTable[i].p1.z;
		m_triangleTable[i].p2.z = -m_triangleTable[i].p2.z;
		m_triangleTable[i].p3.z = -m_triangleTable[i].p3.z;

		m_triangleTable[i].p1.nz = -m_triangleTable[i].p1.nz;
		m_triangleTable[i].p2.nz = -m_triangleTable[i].p2.nz;
		m_triangleTable[i].p3.nz = -m_triangleTable[i].p3.nz;
	}
}


// Returns the pointer to the list of triangles.

void CModFile::SetTriangleUsed(int total)
{
	m_triangleUsed = total;
}

int CModFile::RetTriangleUsed()
{
	return m_triangleUsed;
}

int CModFile::RetTriangleMax()
{
	return MAX_VERTICES;
}

ModelTriangle* CModFile::RetTriangleList()
{
	return m_triangleTable;
}


// Returns the height according to a position (x - z);

float CModFile::RetHeight(Math::Vector pos)
{
	Math::Vector	p1, p2, p3;
	float		limit;
	int			i;

	limit = 5.0f;

	for ( i=0 ; i<m_triangleUsed ; i++ )
	{
		if ( !m_triangleTable[i].bUsed )  continue;

		if ( fabs(pos.x-m_triangleTable[i].p1.x) < limit &&
			 fabs(pos.z-m_triangleTable[i].p1.z) < limit )
		{
			return m_triangleTable[i].p1.y;
		}

		if ( fabs(pos.x-m_triangleTable[i].p2.x) < limit &&
			 fabs(pos.z-m_triangleTable[i].p2.z) < limit )
		{
			return m_triangleTable[i].p2.y;
		}

		if ( fabs(pos.x-m_triangleTable[i].p3.x) < limit &&
			 fabs(pos.z-m_triangleTable[i].p3.z) < limit )
		{
			return m_triangleTable[i].p3.y;
		}
	}

	return 0.0f;
}


