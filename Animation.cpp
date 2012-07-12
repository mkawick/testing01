//animation.cpp

//******************************************
// UNCLASSIFIED
// Copyright © 1997-Pres	Mickey Kawick
// This file may be used anywhere for free, but this
// comment info must be left here.

// If you use this code in any shape or form or even
// as a reference for profit send me a dollar. 
// Not a dollar per copy, just a single US dollar. 

// If you are not in the US, send me
// a few different coins or the equivalent of a dollar.
// I love foreign currency, and I have a little collection.
// MKawick@yahoo.com
//******************************************

// needs maintenence work.

//---------------------------------------------------------------------------

#include "stdafx.h"

#include <list>
using namespace std;

#ifdef _MSC_VER
#include <direct.h>
#else
#include <vcl\vcl.h>
#include <dir.h>
#pragma hdrstop
#endif

#include <dos.h>

#pragma hdrstop

#include "Animation.h"
#include "graphic.h"
#include "GraphicSequence.h"
#include "screen.h"

#ifndef NULL
#define NULL 0L
#endif

//---------------------------------------------------------------------------


			//*********************************
			//*********************************

ANIMATION:: ANIMATION ()
{
    Offset = NULL;
    //Anim = NULL;
    for (int i=0; i<NUMDIR; i++)
    {
        Sequence[i] = NULL;
    }
    Clear();
}

  			//*********************************

ANIMATION:: ANIMATION (STRINGC* path)// init with path, calls Load
{
    Offset = NULL;
    //Anim = NULL;
    for (int i=0; i<NUMDIR; i++)
    {
        Sequence[i] = NULL;
    }
    Clear();
}

  			//*********************************

ANIMATION:: ~ANIMATION ()
{
    Clear ();
}

  			//*********************************

void  ANIMATION:: Clear ()
{
    if (Offset) delete Offset; Offset = NULL;
    //if (Anim) delete Anim; Anim = NULL;
    for (int i=0; i<NUMDIR; i++)
    {
        if (Sequence[i]) delete Sequence[i];
        Sequence[i] = NULL;
    }

    //FilePath.Clear (); FileName.Clear ();

    CurrDirection = EAST;
    NumFiles = 0;
    CurrFrame = 0;
}

			//*********************************
			//*********************************

int   ANIMATION::  CountTotalFiles ()
{
    int count;
    FILE_DIRECTORY filedir;
    STRINGC str = "*.tga";
    //getcwd(cwd, 255);
    chdir (FilePath); // change to the selected directory

    
    //filedir.SetExtension (&str);
    //count = filedir.CountFiles ();
    //Extension = FILEOBJECT :: LLE;


    filedir.SetExtension (str);
    count = filedir.CountFiles ();
    Extension = FILEOBJECT :: TGA;

    if (! count)                      //
    {
        str = "*.bmp";
        filedir.SetExtension (str);
        count = filedir.CountFiles ();
        Extension = FILEOBJECT :: BMP;
    }

    return count;
}

  			//*********************************
//void	GetAllFileNames (ARRAY_DYNAMIC <class STRINGC> & FileList);
void	ANIMATION:: GetAllFileNames (ARRAY_DYNAMIC <class STRINGC> & FileList) const
{
    STRINGC  extn = 0;
 /*   if (Extension == FILEOBJECT :: LLE) extn = "*.lle";
    else   */
    if (Extension == FILEOBJECT :: BMP) extn = "*.bmp";
    else if (Extension == FILEOBJECT :: TGA) extn = "*.tga";

    FILE_DIRECTORY filedirec;
    filedirec.SetExtension (extn);
    filedirec.FindFilesAndGetStrings (FileList);
}

  			//*********************************

// this may not work with GRAPHICSEQUENCE correctly
void  ANIMATION:: LoadSequences (ARRAY_DYNAMIC <class STRINGC> & FileList)   // loads a group of images
{
   /* FileName = str;
    FilePath = str;
    FileName.ExtractName ();
    FilePath.ExtractPath ();
    
    STRINGC* string = &str;// copy to local to retain the pointer
    
    if (string->IsValid ())
    {
        for (int i=0; i<NUMDIR; i++)
        {
            Sequence[i] = new GRAPHICSEQUENCE;
            Sequence[i]->Load (*string, NumFiles, ScratchPad);
            string = string->GetNthItem (NumFiles);
        }
    }*/
	int Num = FileList.GetNum ();
	for (int i=0; i<Num; i++)
	{
//		STRINGC FilePath = FileList[i];
//		Sequence[i] = new GRAPHICSEQUENCE;
        Sequence[i]->Load (FileList, ScratchPad);
	}
}

  			//*********************************
			//*********************************

bool  ANIMATION:: Save (FILE_MASTER& file) // save to a larger file
{
    if (file.Status() != FILE_MASTER :: NONE) return false;
    if (Sequence == NULL) return false;
    if (file.Write (FileName.GetString(), 40)) return false;
    if (file.Write (&NumFiles, sizeof (int))) return false;
    if (file.Write (Offset, sizeof (OFFSET_DATA) * NumFiles)) return false;
    if (file.Write (&AnimType, sizeof (ANIM_DATA :: ANIM_TYPES))) return false;
    if (file.Write (&Extension, sizeof (int))) return false;

    for (int i=0; i<NUMDIR; i++)
    {
        if (Sequence[i]->Save(file)== false) return false;
    }
    return true;
}

  			//*********************************

bool  ANIMATION:: Load (FILE_MASTER& file) // load from a larger file
{
    if (file.Status() != FILE_MASTER :: NONE) return false;
    Clear();
    char str[41];
    if (file.Read(str, 40)) return false;
    FileName = str;
    
    if (file.Read (&NumFiles, sizeof (int))) return false;
        Offset = new OFFSET_DATA [NumFiles];
    if (file.Read (Offset, sizeof (OFFSET_DATA) * NumFiles)) return false;
    if (file.Read (&AnimType, sizeof (ANIM_DATA :: ANIM_TYPES))) return false;
    if (file.Read (&Extension, sizeof (int))) return false;

    for (int i=0; i<NUMDIR; i++)
    {
        if (Sequence[i]->Load(file)== false) return false;
    }
    return true;
}

			//*********************************
			//*********************************

bool  ANIMATION:: LoadFromGraphicDirectory (STRINGC& path, U16ptr scratchpad) // path to a directory
    // containing the files to be counted, divided by 8, and loaded into
    // separate GRAPHICSEQUENCE's
    // also, the name of the actual directory will be parsed to discover the type of
    // anim being loaded
{
    char cwd[255]; // store the current working directory
    //STRINGC* ptr1;
    Clear ();

    FilePath = path; //FilePath.ExtractPath (); // path only
    FileName = path; FileName.ExtractName ();
    ANIMATION_DATA ani;
    AnimType = ani.GetType (FileName);
    getcwd(cwd, 255);
    chdir (FilePath); // change to the selected directory

    ScratchPad = scratchpad;

    int x = CountTotalFiles ();
    if (x == 0) return false;
    if (x % NUMDIR) return false;  // this directory is not divisible by 8
    NumFiles = (U8) (x/NUMDIR);

	ARRAY_DYNAMIC <class STRINGC> FileList;
	GetAllFileNames (FileList);
    //STRINGC* str = GetAllFileNames ();   // this sets up a linked list of filenames
    // that are all of the files in this directory

    LoadSequences (FileList);  // load in groups of 8
//    if (str)   delete str;  // delete the linked list

    chdir (cwd);  // restore the current working directory

    Offset = new OFFSET_DATA[NumFiles];// allot the animation offsets
    return true;
}

			//*********************************
			//*********************************

void  ANIMATION:: DrawAt (SCREEN_STRUCT* screen, int x, int y) const
{
    if (Sequence [CurrDirection] == NULL) return;
    Sequence [CurrDirection]->SetFrame(CurrFrame);
    Sequence [CurrDirection]->SetPosition (x, y);
    Sequence [CurrDirection]->DrawCurrent (screen);
}

			//*********************************
			//*********************************

// based on currframe
void  ANIMATION:: SetOffsetData (const int offx, const int offy, const int offz, const int delay) 
{
    if (Offset == NULL) return;
    //if (Offset[CurrFrame] == NULL) return;
    Offset[CurrFrame].OffX = (S8) offx;
    Offset[CurrFrame].OffY = (S8) offy;
    Offset[CurrFrame].OffZ = (S8) offz;
    Offset[CurrFrame].Delay = (S8) delay;
}

			//*********************************

OFFSET_DATAptr ANIMATION:: GetOffsetData () const 
{
    if (Offset == NULL) return NULL;
    return &Offset[CurrFrame];
}

			//*********************************

void  ANIMATION:: GetOffsetData (int* offx, int* offy, int* offz, int* delay) const 
{
    if (Offset == NULL) return;
    *offx = Offset[CurrFrame].OffX;
    *offy = Offset[CurrFrame].OffY;
    *offz = Offset[CurrFrame].OffZ;
    *delay = Offset[CurrFrame].Delay;
}
			//*********************************

// based on direction
void  ANIMATION:: GetOffsetDataCalculated (float* offx, float* offy, float* offz, int* delay) const 
{
    if (Offset == NULL) return;

  /*  int Angle = 0;
    switch (CurrDirection) // these default angles will need to be modified
    {
        case EAST:
           Angle = trig.GetAngle(0);
        break;

        case SOUTHEAST:
           Angle = trig.GetAngle(315);
        break;

        case SOUTH:
           Angle = trig.GetAngle(270);
        break;

        case SOUTHWEST:
           Angle = trig.GetAngle(225);
        break;

        case WEST:
           Angle = trig.GetAngle(180);
        break;

        case NORTHWEST:
           Angle = trig.GetAngle(135);
        break;

        case NORTH:
           Angle = trig.GetAngle(90);
        break;

        case NORTHEAST:
           Angle = trig.GetAngle(45);
        break;
    }
    *offx = (float)Offset[CurrFrame].OffX*trig.cose(Angle) - (float)Offset[CurrFrame].OffY*trig.sine(Angle);
    *offy = (float)Offset[CurrFrame].OffY*trig.cose(Angle) + (float)Offset[CurrFrame].OffX*trig.sine(Angle);
    *offz = (float)Offset[CurrFrame].OffZ;
    *delay = (int)Offset[CurrFrame].Delay;*/
}

			//*********************************

int   ANIMATION:: GetDelay (const int whichframe) const 
{
    if (whichframe > NumFiles)  return 0;
    return (int) Offset[whichframe].Delay;
}


			//*********************************
			//*********************************



           
			//*********************************
			//*********************************

ANIMATION_MGR :: ANIMATION_MGR () : ANIMSCREEN_POS()
{
    Anim = new ANIMATIONptr [MAXANIMATION];
    for (int i=0; i<MAXANIMATION; i++)
        Anim[i] = NULL;
    Clear ();
}

			//*********************************

ANIMATION_MGR :: ~ANIMATION_MGR ()
{
    Clear ();
    delete Anim;
}

			//*********************************
			//*********************************

void  ANIMATION_MGR :: Clear ()
{
    CurrDirection = ANIMATION :: EAST;
    CurrAnim = 0;
    CurrFrame = 0;
    FrameCount = 0;
    FileName.Clear();
    for (int i=0; i<MAXANIMATION; i++)
    {
        if (Anim[i]) {delete Anim[i]; Anim[i] = NULL;}
    }
}

			//*********************************

int  ANIMATION_MGR :: CountAnims ()
{
    NumAnim = 0;
    for (int i=0; i<MAXANIMATION; i++)
    {
        if (Anim[i]) NumAnim++;
    }
    return NumAnim;
}

			//*********************************
			//*********************************

void  ANIMATION_MGR :: NextFrame ()// advance the frame and wrap
{
    FrameCount++;
    if (FrameCount <= Anim[CurrAnim]->GetDelay (CurrFrame)) return;
    FrameCount = 0;
    CurrFrame ++;
    int max = Anim[CurrAnim]->GetNumFiles();
    if (CurrFrame >= max) CurrFrame = 0;
}

			//*********************************

void  ANIMATION_MGR :: PrevFrame () // reverse frame, does not wrap
{
    FrameCount++;
    if (FrameCount <= Anim[CurrAnim]->GetDelay (CurrFrame)) return;
    FrameCount = 0;
    CurrFrame--;
    int max = Anim[CurrAnim]->GetNumFiles();
    if (CurrFrame < 0) CurrFrame = max-1;
}

			//*********************************
           //*********************************

bool  ANIMATION_MGR :: Load (STRINGC& path, U16ptr ScratchPad)
    // this path is in the directory where all of
    // the animation subdirectories lie. The path may read something like this:
    // c:\\temp\\goblin\\blah.txt
    // if the extension is ".ani" then the Load (FILE_MASTER* file) is called to
    // open and read this file
    // all other extensions are read as simply a pointer to the subdir and the file
    // name passed is discarded. Then the parent directory of the filename is used as
    // the "FileName" for the ANIMATION_MGR.
{
    Clear ();

    STRINGC temp = path;
    temp.ConvertBackslash ();
    STRINGC name = temp;
    
    temp.ExtractExtension ();
    if (temp == "ani") return false;// actually load from a file // may never
                                   // be used

    name.ExtractPath ();     // we now have the path
    FileName = name;
    FileName.ExtractName (); // just the parent directory name

    FILE_DIRECTORY File;
    File.SetSubDirectoryFlag ();  // looking for directories only

	ARRAY_DYNAMIC <class STRINGC> DirList;
	File.FindFilesAndGetStrings (DirList);  // get directory names
    //STRptr str = File.FindFilesAndGetStrings (&name);  // get directory names

    //STRptr string = str;

	int Num = DirList.GetNum ();
	for (int i=0; i<Num; i++)
	{
		STRINGC Dir = DirList[i];
		Anim[i] = new ANIMATION;
        Anim[i]->LoadFromGraphicDirectory (Dir, ScratchPad);
	}
   /* int count = 0;
    while (string != NULL)
    {
        Anim[count] = new ANIMATION;
        Anim[count]->LoadFromGraphicDirectory (*string, ScratchPad);
//        string = string->GetNext ();
        count++;
    }*/
    return true;
}

			//*********************************

bool ANIMATION_MGR :: Load (FILE_MASTER& file)
{
    Clear ();
    char str[41];
	if (file.Status() != FILE_MASTER::NONE) return false;
    file.Read (str, 40);
    file.Read (& NumAnim, sizeof (int));
    for (int i=0; i<NumAnim; i++)
    {
        if (Anim[i])
        Anim[i]->Load (file);
    }
    FileName = str;
    return true;
}

			//*********************************

bool ANIMATION_MGR :: Save (FILE_MASTER& file)
{
    CountAnims ();
    const char* str = FileName.GetString ();
	if (file.Status() != FILE_MASTER::NONE) return false;
    file.Write (str, 40);
    file.Write (&NumAnim, sizeof (int));
    for (int i=0; i<MAXANIMATION; i++)
    {
        if (Anim[i])
        Anim[i]->Save (file);
    }
    return true;
}

			//*********************************
			//*********************************

void  ANIMATION_MGR :: SetAnimType (ANIMATION_DATA :: ANIM_TYPES type)
{
    for (int i=0; i<MAXANIMATION; i++)
    {
        if (Anim[i])
        {
           if (Anim[i]->GetAnimType() == type)
           {
              CurrAnim =  i;
              return;
           }
        }
    }
}

			//*********************************

ANIMATION_DATA :: ANIM_TYPES
ANIMATION_MGR :: GetAnimType () const 
{
    if (Anim[CurrAnim])
        return Anim[CurrAnim]->GetAnimType ();
    return ANIMATION_DATA :: NONE;
}

			//*********************************  
			//*********************************

void  ANIMATION_MGR :: SetOffsetData (const int offx, const int offy, const int offz, 
									  const int delay)
{
    if (Anim[CurrAnim])
    Anim[CurrAnim]->SetOffsetData (offx, offy, offz, delay);
}

			//*********************************

void  ANIMATION_MGR :: GetOffsetData (int* offx, int* offy, int* offz, int* delay) const
{
    if (Anim[CurrAnim])
    Anim[CurrAnim]->GetOffsetData (offx, offy, offz, delay);
}

			//*********************************
			//*********************************

void  ANIMATION_MGR :: Draw (SCREEN_STRUCT* screen) 
{
    if (Anim[CurrAnim])
    {
        int max = Anim[CurrAnim]->GetNumFiles();
        if (CurrFrame >= max) CurrFrame = 0;
        Anim[CurrAnim]->SetDirection (CurrDirection);
        Anim[CurrAnim]->SetFrame (CurrFrame);
        Anim[CurrAnim]->DrawAt (screen, (int)scx, (int)scy);
    }
}

			//*********************************
			//*********************************

void  ANIMATION_MGR :: DrawAt (SCREEN_STRUCT* screen, int x, int y) 
{
	if (Anim[CurrAnim])
    {
        int max = Anim[CurrAnim]->GetNumFiles();
        if (CurrFrame >= max) CurrFrame = 0;
        Anim[CurrAnim]->SetDirection (CurrDirection);
        Anim[CurrAnim]->SetFrame (CurrFrame);
        Anim[CurrAnim]->DrawAt (screen, x, y);
    }
}

			//*********************************
			//*********************************

