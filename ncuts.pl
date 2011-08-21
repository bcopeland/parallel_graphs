#! /usr/bin/perl

use strict;

die "Usage: ncuts.pl graph part" if (@ARGV < 2);

my $graphfile = shift @ARGV;
my $partfile = shift @ARGV;

my %parts;
my %pedges;
my %pcrosses;

# read partition file
my $idx = 1;
my @verts;
open (F1, $partfile);
while(<F1>)
{
    chomp;
    $verts[$idx++] = $_;
    $parts{$_}++;
}
close(F1);

# now read graph and count times the edge crosses a partition
open (F2, $graphfile);
my $header=<F2>;

my ($nverts, $nedges) = split(/ /, $header);

$idx = 1;
my ($cuts, $nadj, $edges) = 0;
while (<F2>)
{
    chomp;
    my %adj = ();
    foreach my $neighbor (split(/ /))
    {
        if ($verts[$neighbor] != $verts[$idx])
        {
            $cuts++;
            $adj{$neighbor} = 1;
            $pedges{$verts[$idx]}->{out}++;
            $pedges{$verts[$neighbor]}->{in}++;
        }
        else
        {
            $pedges{$verts[$idx]}->{res}++;
        }
        $edges++;
        # edges originating in this partition
    }
    $nadj += scalar keys(%adj);
    $idx++;
}
print "cuts: $cuts edges: $edges " . ($cuts/$edges) . "\n";

# totalv: sum of domains of cuts.. this counts something
# like ghost cells?
print "totalv: $nadj\n";

my $max_part = 0;
foreach my $key (keys(%parts))
{
    print "cpu$key: ", $parts{$key}, " verts ",
        $pedges{$key}->{res}, " internal, ", 
        $pedges{$key}->{in}, " inbound, ", 
        $pedges{$key}->{out}, " outbound edges\n";

    if ($parts{$key} > $max_part)
    {
        $max_part = $parts{$key};
    }
}
my $nparts = scalar keys(%parts);
print "balance: ", ($nparts * $max_part/$nverts), "\n";


