#!/usr/bin/perl -wT

use strict;
$|=1;

if ($#ARGV<1) {
  print STDERR "usage: rcrc RCFILE OUTFILE\n";
  exit 1;
}

my $sInputfile=shift;
my $sOutputfile=shift;

open(IN,"<$sInputfile") or die "fatal: cannot open $sInputfile\n";
$sOutputfile=~s/[^0-9a-zA-Z_:\.\-]/_/g;
$sOutputfile=$1 if $sOutputfile=~/(.*)/;
open(OUT,">$sOutputfile") or die "fatal: cannot create $sOutputfile\n";

print OUT <<"EOF" ;
/* ======================================================================
* resource file created by rcrc (Marian Eichholz 4.4.2003)
* from: $sInputfile
*   to: $sOutputfile
* ====================================================================== */

EOF

# ======================================================================
# Scanner/Lexer part
# ======================================================================

my $sPendingToken;
my $sValue;
my $ch;
my $iLine=0;
my $sLastLine="(unread)";

sub dlog {
  # print "INFO: ",@_;
}

sub Panic {
  print STDERR "rc file error at line ",$iLine,": ";
  print STDERR @_;
  print STDERR " [",$sLastLine,"]\n";
  exit 2
}

my $chPending;

my %hSymbols;
my %hTokens=( "MENU",1, "BEGIN",2, "END",3, "MENUITEM",4,
	      "SEPARATOR",5, "SUBMENU",6, "LASTMENU",7 );

# ----------------------------------------------------------------------

sub ProcessIncludeFile {
  my $sIncludeName=shift;
  my $iIncludeLine=0;
  open (INCLUDE,"<$sIncludeName") or Panic("cannot open include file");
  while (<INCLUDE>)
    {
      chop;
      if (/^#define\s+([a-zA-Z_][a-zA-Z_0-9]*)\s+(\S.*)/) {
	{
	  dlog("defining $1 as [$2]\n");
	  $hSymbols{$1}=$2;
	}
      }
    }
  close(INCLUDE);
  return 1;
}

# ----------------------------------------------------------------------

my $sPending;
sub GetChar() {
  if (defined $chPending)
    {
      $ch=$chPending;
      $chPending=undef();
      return 1;
    }
  if ($sPending eq "") {
    return 0 if eof(IN);
    $iLine++;
    $sPending=<IN>;
    $sLastLine=$sPending;
    chop $sLastLine;
    if ($sPending=~/^#include\s+"([0-9a-zA-Z-_\.+~]+)"\s+(.*)/)
      {
	ProcessIncludeFile($1);
	$sPending=$2."\n";
      }
  }
  $ch=substr($sPending,0,1);
  $sPending=substr($sPending,1);
  return 1;
}

# ----------------------------------------------------------------------

sub EatWS {
 do {
   return 0 unless GetChar();
  } while $ch=~/\s/;
 return 1;
}

# ----------------------------------------------------------------------

sub GetString {
  $sValue="";
  my $iLine0=$iLine;
  do {
    Panic("unexpected EOF, runaway String at ",$iLine0) unless GetChar();
    $sValue=$sValue.$ch unless $ch eq '"';
  } while $ch ne "\"";
  return "#string";
}

# ----------------------------------------------------------------------

sub GetSymbol {
  my $s=$ch;
  while (GetChar() and $ch=~/[a-zA-Z0-9_]/)
    {
      $s=$s.$ch;
    }
  $chPending=$ch; # stow back last character
  return $s if $hTokens{$s};
  $sValue=$s;
  return "#symbol";
}

# ----------------------------------------------------------------------

sub ReponeToken {
  $sPendingToken=shift;
}

# ----------------------------------------------------------------------

sub GetToken {
  if (defined($sPendingToken))
    {
      my $s=$sPendingToken;
      $sPendingToken=undef();
      return $s;
    }
  # leading spaces away
  $sValue="";
  $sPendingToken=undef();
  EatWS() or return "";
  if ($ch eq "#") {
    while (GetChar())
      {
	return "#comment" if $ch eq "\n";
      }
    return "#comment";
  } # if comment
  # now get the real token
  my $endsym='[\s#]';
  return GetString() if $ch eq '"';
  return GetSymbol() if $ch=~/^[a-zA-Z]/;
  $sValue=$ch;
  return "#char";
}

# ======================================================================
# Parsing part as direct descending parser
# ======================================================================

my @aMenuTitle;

sub ParseMenuitem {
  my ($iDepth,$iPos)=@_;
  my $sToken=GetToken();
  my $sID=$sValue;
  if ($sToken eq "SEPARATOR")
    {
      print OUT "\t{ \"-\",NULL,ID_SEPARATOR, $iDepth,$iPos },\n";
    }
  else
    {
      Panic("item ID expected") if $sToken ne "#num" and $sToken ne "#symbol";
      # TODO: should check against definitions / uniqueness
      Panic("item string expected") unless GetToken() eq "#string";
      my $sTitle=$sValue;
      my $sToken=GetToken();
      my $sAccel="NULL";
      if ($sToken eq "#char" and $sValue eq ",")
	{
	  Panic("accel string expected") unless GetToken() eq "#string";
	  $sAccel="\"$sValue\"";
	}
      else
	{
	  ReponeToken($sToken);
	}
      print OUT "\t{ \"$sTitle\", $sAccel, $sID, $iDepth, $iPos },\n";
    }
}

sub ParseSubmenu {
  my $iDepth=shift;
  Panic("menu title expected") unless GetToken() eq "#string";
  my $sMenuTitle=$sValue;
  my $sRawMenuTitle=$sValue; $sRawMenuTitle =~ s/_//g;
  my $sItem=GetToken();
  my $bLastMenu=0;
  $aMenuTitle[$iDepth]=$sMenuTitle;
  if ($sItem eq "LASTMENU") {
    $bLastMenu=1;
    $sItem=GetToken();
  }
 my $iItem=0;
 Panic("MENU: BEGIN expected") unless $sItem eq "BEGIN";
  do {
    $sItem=GetToken();
    if ($sItem eq "END") { dlog("found end-of-submenu <$sMenuTitle>\n"); }
    elsif ($sItem eq "MENUITEM") { ParseMenuitem($iDepth,$iItem); }
    elsif ($sItem eq "SUBMENU") { ParseSubmenu($iDepth+1,$iItem); }
    else { Panic("submenu or item expected"); }
    $iItem++;
  } while $sItem ne "END";
}

sub ParseMenu {
  Panic("menu id expected") unless GetToken() eq "#symbol";
  my $sMenuID=$sValue;
  Panic("MENU: BEGIN expected") unless GetToken() eq "BEGIN";
  my $sItem;
  print OUT "\nstatic struct TMenuTemplateItem ",$sMenuID,"[] = {\n";
  do {
    $sItem=GetToken();
    if ($sItem eq "END") { dlog("found end-of-menu\n"); }
    elsif ($sItem eq "SUBMENU") { ParseSubmenu(0,0) }
    else { Panic("SUBMENU or END expected"); }
  } while $sItem ne "END";
  print OUT "}; /* end of $sMenuID */\n";
  return 1;
}

$sPending=""; # initialise scanner
my $sRessource;
while ($sRessource=GetToken())
  {
    dlog("found: $sRessource\n");
    
    if ($sRessource eq "MENU") { ParseMenu() }
    elsif ($sRessource eq "#comment") { }
    else { Panic("MENU expected"); }
  }

close IN;

# exit 1;
