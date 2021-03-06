{
    This file is part of the Free Pascal run time library.
    Copyright (c) 1999-2000 by Florian Klaempfl and Pavel Ozerski
    member of the Free Pascal development team.

    FPC Pascal system unit for the Win32 API.

    See the file COPYING.FPC, included in this distribution,
    for details about the copyright.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 **********************************************************************}

{*****************************************************************************
                           Directory Handling
*****************************************************************************}
type
 TDirFnType=function(name:pointer):longbool;stdcall;

function CreateDirectoryTrunc(name:pointer):longbool;stdcall;
begin
  CreateDirectoryTrunc:=CreateDirectoryW(name,nil);
end;

procedure dirfn(afunc : TDirFnType;s:unicodestring);
begin
  DoDirSeparators(s);
  if not aFunc(punicodechar(s)) then
    begin
      Errno2InoutRes(GetLastError);
    end;
end;

Procedure do_MkDir(const s: UnicodeString);
begin
  dirfn(TDirFnType(@CreateDirectoryTrunc),s);
end;

Procedure do_RmDir(const s: UnicodeString);
begin
  if (s ='.') then
    begin
      InOutRes := 16;
      exit;
    end;
  {$ifdef WINCE}
  if (s='..') then
    begin
      InOutRes := 5;
      exit;
    end;
  {$endif WINCE}
  dirfn(TDirFnType(@RemoveDirectoryW),s);
{$ifdef WINCE}
  if (Inoutres=3) and (Pos(DirectorySeparator, s)<2) then
    Inoutres:=2;
{$endif WINCE}
end;

Procedure do_ChDir(const s: UnicodeString);
{$ifndef WINCE}
var
  EnvName: array [0..3] of WideChar;
  Len, Len2: cardinal;
  FullPath: UnicodeString;
  P: PWideChar;
{$ENDIF WINCE}
begin
{$ifndef WINCE}
  Len := GetFullPathNameW (PUnicodeChar (S), 0, nil, P); // in TChar
  SetLength (FullPath, Len - 1); // -1 because len is #0 inclusive
  Len2 := GetFullPathNameW (PUnicodeChar (S), Len, PUnicodeChar (FullPath), P);
  if Len2 <> 0 then
   begin
(* Remove potential trailing backslashes *)
    while (Len2 > 3) and (FullPath [Len2] = WideChar ('\')) do
     Dec (Len2);
    if Len2 <> Len - 1 then
{ Real length returned by GetFullPathNameW seems to be usually smaller than originally requested buffer size }
     SetLength (FullPath, Len2);
{ Use FullPath for SetCurrentDirectory instead of original input to ensure consistency }
    DirFn (TDirFnType (@SetCurrentDirectoryW), FullPath);
    if (InOutRes = 0) and (Length (S) > 2) and (S [2] = ':') then
     begin
      EnvName [0] := '=';
      EnvName [1] := S [1];
      EnvName [2] := ':';
      EnvName [3] := #0;
      SetEnvironmentVariableW (@EnvName, PUnicodeChar (FullPath));
     end
   end
  else
{ Try SetCurrentDirectoryW with the original input if GetFullPathNameW errors out }
   dirfn(TDirFnType(@SetCurrentDirectoryW),s);
  if Inoutres=2 then
   Inoutres:=3;
{$else WINCE}
  InOutRes:=3;
{$endif WINCE}
end;

procedure do_GetDir (DriveNr: byte; var Dir: Unicodestring);
{$ifndef WINCE}
var
  Drive:array[0..3]of widechar;
  P: PWideChar;
  Len, Len2: cardinal;
{$endif WINCE}
begin
{$ifndef WINCE}
  if DriveNr <> 0 then
   begin
    Drive[0]:=widechar(DriveNr+ Ord ('A') - 1);
    Drive[1]:=':';
    Drive[2]:=#0;
    Drive[3]:=#0;
    Len := GetFullPathNameW (@Drive, 0, nil, P); // in TChar
    SetLength (Dir, Len - 1); // -1 because len is #0 inclusive

    Len2 := GetFullPathNameW (@Drive, Len, PUnicodeChar (Dir), P);
    if Len2 = 0 then
     begin
      Errno2InoutRes(GetLastError);
      Dir := widechar (DriveNr + Ord ('A') - 1) + ':\';
      Exit;
     end
    else
     begin
{ Real length returned by GetFullPathNameW seems to be usually smaller than originally requested buffer size }
      if Len2 <> Len - 1 then
       SetLength (Dir, Len2);
      if not FileNameCasePreserving then
       Dir := UpCase (Dir);
     end;
   end
  else
   begin
    Len := GetCurrentDirectoryW (0,nil);
    SetLength (Dir, Len - 1); // -1 because len is #0 inclusive
    GetCurrentDirectoryW (Len, PUnicodeChar (Dir));
    if not FileNameCasePreserving then
     Dir := UpCase (Dir);
   end;
{$else WINCE}
  Dir:='\';
{$endif WINCE}
end;
