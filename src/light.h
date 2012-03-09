﻿// * This file is part of the COLOBOT source code
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
// * along with this program. If not, see .

#ifndef _LIGHT_H_
#define	_LIGHT_H_



class CInstanceManager;
class CD3DEngine;


#define D3DMAXLIGHT		100


typedef struct
{
	float	starting;
	float	ending;
	float	current;
	float	progress;
	float	speed;
}
LightProg;


typedef struct
{
	char			bUsed;			// TRUE -> lumi�re existe
	char			bEnable;		// TRUE -> lumi�re allum�e

	D3DTypeObj		incluType;		// type de tous les objets inclus
	D3DTypeObj		excluType;		// type de tous les objets exclus

	D3DLIGHT7		light;			// configuration de la lumi�re

	LightProg		intensity;		// intensit� (0..1)
	LightProg		colorRed;
	LightProg		colorGreen;
	LightProg		colorBlue;
}
Light;



class CLight
{
public:
	CLight(CInstanceManager *iMan, CD3DEngine* engine);
	~CLight();

	void			SetD3DDevice(LPDIRECT3DDEVICE7 device);

	void			FlushLight();
	int				CreateLight();
	BOOL			DeleteLight(int lightRank);
	BOOL			SetLight(int lightRank, const D3DLIGHT7 &light);
	BOOL			GetLight(int lightRank, D3DLIGHT7 &light);
	BOOL			LightEnable(int lightRank, BOOL bEnable);

	BOOL			SetLightIncluType(int lightRank, D3DTypeObj type);
	BOOL			SetLightExcluType(int lightRank, D3DTypeObj type);

	BOOL			SetLightPos(int lightRank, D3DVECTOR pos);
	D3DVECTOR		RetLightPos(int lightRank);

	BOOL			SetLightDir(int lightRank, D3DVECTOR dir);
	D3DVECTOR		RetLightDir(int lightRank);

	BOOL			SetLightIntensitySpeed(int lightRank, float speed);
	BOOL			SetLightIntensity(int lightRank, float value);
	float			RetLightIntensity(int lightRank);
	void			AdaptLightColor(D3DCOLORVALUE color, float factor);

	BOOL			SetLightColorSpeed(int lightRank, float speed);
	BOOL			SetLightColor(int lightRank, D3DCOLORVALUE color);
	D3DCOLORVALUE	RetLightColor(int lightRank);

	void			FrameLight(float rTime);
	void			LightUpdate();
	void			LightUpdate(D3DTypeObj type);

protected:

protected:
	CInstanceManager* m_iMan;
	CD3DEngine*		  m_engine;
	LPDIRECT3DDEVICE7 m_pD3DDevice;

	float			m_time;
	int				m_lightUsed;
	Light*			m_lightTable;
};


#endif //_LIGHT_H_
