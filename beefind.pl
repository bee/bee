#!/usr/bin/perl -w

use Data::Dumper;
use File::Find;
use Time::HiRes;
use Digest::MD5;
use File::stat;
use Fcntl;
use Getopt::Long;

$Data::Dumper::Indent=1;
$Data::Dumper::Terse=1;
my @STATNAME=(qw(dev ino mode nlink uid gid rdev size atime mtime ctime blksize blocks));

my %HARDLINK;
my @HARDLINKS;

my $t0 = Time::HiRes::time;

my @MD5SKIP = qw( pipe socket block char tty );
my %MD5SKIP = map {($_ => undef)} @MD5SKIP;

my $ls;

my %opt = ( 
   maxdepth => -1,
   md5      => 1,
   xdev     => 0,
   help     => 0,
   filelist => undef,
   skiplist => undef,
   dumpfilelist => 0,
   format => 'mxpkg',
   exclude => [],
   cutroot => '',
   ignore => 'ino',
   ignore_dirmtime => 0,
   cat => 0,
   
);

sub help {
  print <<"--EOH--";
  
$0 [--nomd5] [--xdev] [--maxdepth=LEVELS] PATH..
$0 [--nomd5] --from-file=FILE

    --nomd5            don't calculate md5sum for files
    --xdev             don't descend directories on other filesystems
    --maxdepth=LEVELS  descend at most LEVELS levels of directories
    --from-file=FILE   get names to process from file FILE
    --ignore_dirmtime  ignore mtime on directories

$0 --dumpfilenames [--format=FORMAT] [LISTFILE]  

    FORMAT=mxpkg    (default) 
    FORMAT=find-ls
    
  
--EOH--
exit;
}

$result = GetOptions ("maxdepth=i"       => \$opt{maxdepth},
                      "md5!"             => \$opt{md5},
		      "xdev!"            => \$opt{xdev},
		      "dumpfilenames"    => \$opt{dumpfilelist},
		      "format=s"         => \$opt{format},
		      "help"             => \$opt{help},
		      "ignore=s"         => \$opt{ignore},
		      "ignore_dirmtime!" => \$opt{ignore_dirmtime},
		      "files-from|from-file=s"      => \$opt{filelist},
		      "excludelist=s"       => \$opt{excludelist},
		      "exclude=s"        => $opt{exclude},
		      "cutroot=s"        => \$opt{cutroot},
		      "cat!" => \$opt{cat},
		      );

help() if(not $result or $opt{help});

if($opt{cat}){

  my ($f, @f);

  my $filelist = shift @ARGV;

  $filelist = '-' unless($filelist);
  die "can't open $filelist: $!\n" unless(open(FH, "<$filelist"));

  my %ignore = map {( $_ => undef)} split/,/, $opt{ignore};

  foreach(<FH>) {
    chomp;
    
    unless(($f) = /(:file=.*?)$/) {
      die "invalid file-format..\n";
    }

#    s/\n//g;
    s/:file=.*?$//;

    my $x = join ":", grep { /^(\S+)=/ && not exists $ignore{$1} } split /:/;
    
    print "$x$f\n";

  }
  
  
  exit;
}





#print STDERR Dumper \%opt;

if($opt{dumpfilelist}) {
  my $filelist = shift @ARGV;
  $filelist = '-' unless($filelist);
  die "can't open $filelist: $!\n" unless(open(FH, "<$filelist"));
  foreach(<FH>) {
     chomp;
     if($opt{format} eq 'find-ls') {
       my $s=10;
       s/^\s*//g;
       my @L = split /\s+/, $_;
       if($L[2] =~ /^[bc]/) {
          $s = 9;
       }
       $_ = join " ", @L[$s..$#L];
       s, -> .*$,,;
       print "$_\n";
     } elsif(/:file=(.*?)$/) {
        $_ = $1;
	s,//.*$,,g;
	print "$_\n";
     } else {
       print STDERR "invalid format detected:\n  *** $_\n";
     }
  }  
  exit;
}

if($opt{filelist}) {
  die "can't open $opt{filelist}: $!\n" unless(open(FH, "<$opt{filelist}"));
  foreach(<FH>) {
     chomp;
     unless($ls = lstat($_)){
      warn "can't stat '$_': $!\n";
      next
      };
     print file2output($_);
  }
  close FH;
  exit;
}

if(@ARGV) {
  scan(@ARGV);
} else {
  help();
}




sub scan_preprocess { 
  $__scan_DEPTH++; 
  return if($opt{maxdepth} >= 0 and $__scan_DEPTH > $opt{maxdepth});
#  printf STDERR "[$__scan_DEPTH] scanning $File::Find::dir ..\n";
  return sort @_;
}

sub scan_postprocess { 
#  printf STDERR "[$__scan_DEPTH] finished scanning of $File::Find::dir ..\n" 
#		        unless($opt{maxdepth} >= 0 and $__scan_DEPTH > $opt{maxdepth});
  $__scan_DEPTH--; 
}

sub scan {

  if($opt{excludelist}) {
    open FH,$opt{excludelist} or die qq(can't open $opt{excludelist}: $!\n);
    foreach(<FH>) {
      chomp;
      next if(/^\s*$/);
      s/\s*$//;
      
      push @SKIP, $_;
    }
    close FH;
  }
  
  push @SKIP, @{$opt{exclude}};
  
#  print STDERR Dumper \@SKIP, $opt{exclude};
  
  File::Find::find({ 
     wanted      => \&scan_process,
     preprocess  => \&scan_preprocess,
     postprocess => \&scan_postprocess,
     no_chdir    => 1
   }, @_);
}


sub scan_process {

     my $file = $File::Find::name;
     
     $ls = lstat($file) or die "can't stat '$file': $!\n";

     if($opt{xdev} && ($File::Find::prune |= ($ls->dev != $File::Find::topdev))) {
        printf STDERR "skipping $file .. $File::Find::topdev != ". $ls->dev . "\n";
	print file2output($file);
	return 0;
     }


     return 0 if($file eq $opt{cutroot});
     
     foreach(@SKIP) {
       my $f = $file;
       $f =~ s/^\Q$opt{cutroot}\E//;
       if($f =~ qr(${_})) {
         printf STDERR "skipping $file ($f) .. in SKIP($_).. \n";
	 return 0;
       }
     }

     if($File::Find::prune |= (exists $SKIP{$file})) {
        printf STDERR "skipping $file .. in SKIP.. \n";
	print file2output($file);
	return 0;
     }

     print file2output($file);
     return 1;
}

sub file2output {
     my $file = shift;
     my $md5='off';
     my $link='';
     my $islink = 0; 

     if (-f _ && $ls->nlink > 1) {
	my $index = $ls->dev. "#" . $ls->ino; 

	if (exists $HARDLINK{$index}) {
	   $islink = 1; 
	}
	push @{$HARDLINK{$index}},$file;
     }

     
     if (-f _ ) {
        if($opt{md5}) {
	  if(sysopen(FILE, "$file", O_RDONLY)) {
	    binmode(FILE);
	    $md5 = Digest::MD5->new->addfile(*FILE)->hexdigest();
	    close(FILE);
	  } else {
	    warn("Can't open '$file': $!\n");
	    $md5 = '#MD5OPENERROR#';
	  }
	}
     }

     elsif (-d _) { $md5='directory'; } # plain directory
     elsif (-l _) { $md5='link'; $link = '//'.readlink($file); } # plain symlink
     elsif (-p _) { $md5='pipe'; } # plain pipe
     elsif (-S _) { $md5='socket'; } # plain socket
     elsif (-b _) { $md5='block'; } # plain block special
     elsif (-c _) { $md5='char'; } # plain character special
     elsif (-t _) { $md5='tty'; } # plain tty
     else {
	print STDERR "#?#:$file (UNKNOWN)\n";
     }
     
     if(exists $MD5SKIP{$md5}) {
#       print STDERR "md5=$md5:file=$file (in MD5SKIP .. ignored)\n";
       return ""
     }
     
     my @f = qw(ino mode nlink uid gid size mtime);
     my %ignore = map {( $_ => undef)} split/,/, $opt{ignore};
     
     if($md5 eq 'directory' and $opt{ignore_dirmtime}) {
        $ignore{size}  = undef;
        $ignore{mtime} = undef;
	$ignore{nlink} = undef;
     }
     if($md5 eq 'link') {
        $ignore{nlink} = undef;
     }
     
     @f = grep {not exists $ignore{$_}} @f;

     my @x = map {"$_=" . eval '$ls->$_'} @f;
     
     $file =~ s/^\Q$opt{cutroot}\E//;

     return join(':', "md5=$md5", @x, "file=$file$link\n");
}


