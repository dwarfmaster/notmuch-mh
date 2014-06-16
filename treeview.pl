#!/usr/bin/perl

use strict;
use warnings;
use utf8;
use JSON qw(decode_json);
use Getopt::Std;
use Term::ANSIColor;
$Term::ANSIColor::AUTORESET=1;

# Config
my %opts;
getopt("Dsf", \%opts);
my $spaces  = $opts{"s"};
$spaces = 1 if not $spaces;
my $dup     = $opts{"d"};
my $stripre = $opts{"r"};
my $mdepth  = $opts{"D"};
my $ascii   = $opts{"A"};
my $format  = $opts{"f"};
$format = "%{bold cyan}%sj %{green}[%fr]" if not $format;

# String to draw
my $barvert = "|";
my $barhori = "-";
my $angle   = "\\";
my $new     = "|";
my $first   = "+";
my $arrow   = ">";
if(not $ascii) {
    $barvert = "\x{2502}";
    $barhori = "\x{2500}";
    $angle   = "\x{2514}";
    $new     = "\x{251c}";
    $first   = "\x{252c}";
    $arrow   = "\x{25b6}";
    binmode(STDOUT, ":utf8");
}

my $json;
$json  = get_input();
foreach my $thread (@$json) {
    parse_level(0, $new, "", @$thread[0]);
}

sub get_input {
    my $text;
    while(<STDIN>) {
        $text = "$text$_";
    }
    return decode_json($text);
}

sub parse_level {
    my ($dec,$lnew,$prev,$json,@symbs) = @_;
    return if $mdepth and $dec >= $mdepth;
    my ($mail,@subs) = parse_headers($json);

    my $i;
    for($i = 0; $i < $dec - 1; ++$i) {
        print "$symbs[$i]";
        for(my $j = 0; $j < $spaces; ++$j) {
            print " ";
        }
    }

    print "$lnew" if $dec >= 1;
    if(scalar(@symbs) > 0) {
        for(my $j = 0; $j < $spaces; ++$j) {
            print "$barhori";
        }
    }
    if(scalar(@subs) > 0) {
        print "$first";
    } else {
        print "$barhori";
    }

    print "$arrow ";
    my $subject = print_string($mail, $prev);

    push @symbs, $barvert;
    for($i = 0; $i < scalar(@subs) - 1; ++$i) {
        parse_level($dec+1,$new,$subject,$subs[$i],@symbs);
    }
    $symbs[-1] = " ";
    parse_level($dec+1,$angle,$subject,$subs[-1],@symbs) if scalar(@subs) > 0;
}

sub parse_headers {
    my ($json) = @_;
    my $item = @$json[0];

    my $mail = $item;
    my $subs = @$json[1];

    return ($mail, @$subs);
}

sub strip_re {
    my ($mail) = @_;
    while($mail =~ m/^Re: (.*)$/ or $mail =~ m/^Re:(.*)$/) {
        $mail = $1;
    }
    return $mail;
}

sub print_string {
    my ($mail,$prev) = @_;
    my $text = $format;

    my $tags = "";
    foreach my $tag ($mail->{tags}) {
        $tags = "$tags $tag";
    }
    $text =~ s/([^\\])%to/$1$mail->{headers}{To}/g;
    $text =~ s/([^\\])%fr/$1$mail->{headers}{From}/g;
    $text =~ s/([^\\])%cc/$1$mail->{headers}{Cc}/g;
    $text =~ s/([^\\])%dt/$1$mail->{headers}{Date}/g;
    $text =~ s/([^\\])%rt/$1$mail->{date_relative}/g;
    $text =~ s/([^\\])%tt/$1$mail->{timestamp}/g;
    $text =~ s/([^\\])%tg/$1$tags/g;
    $text =~ s/([^\\])%fl/$1$mail->{filename}/g;

    my $subject = $mail->{headers}{Subject};
    $subject = strip_re($subject) if $stripre;
    if($dup or $subject ne $prev) {
        $text =~ s/([^\\])%sj/$1$subject/g;
    } else {
        $text =~ s/([^\\])%sj/$1/g;
    }
    $text =~ s/\\%/%/g;

    my @parts = split /%{/, $text;
    foreach my $part (@parts) {
        next if $part !~ m/^(.*)}(.*)$/;
        print colored [$1], $2;
    }
    print "\n";

    return $subject;
}



