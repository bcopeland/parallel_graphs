#! /usr/bin/perl

sub max
{
    my ($a, $b) = @_;
    return ($a < $b) ? $b : $a;
}

my $graph = "oleo/twitter_graph_max";
my @algorithms = 
(
    "rand", "rr", "pmetis", "gmin", "gmax"
);

my $fn = "$graph.trace.bfs.nopart.1";
open($fh, $fn) || die "$! opening $fn";
my $single_proc = parse_trace($fh, $algo);
close($fh);

my $fn = "oleo/ec2.bfs.nopart.1";
open($fh, $fn) || die "$! opening $fn";
my $ec2_single_proc = parse_trace($fh, $algo);
close($fh);

foreach my $algo (@algorithms)
{
    my @results = ();

    for (my $i=2; $i <= 4; $i++)
    {
        my $fh;
        my $fn = "$graph.trace.bfs.$algo.$i";
        open($fh, $fn) || die "$! opening $fn";
        push @results, parse_trace($fh, $algo);
        close($fh);
    }
    open($fh, ">$graph.$algo.time.dat");
    print $fh "1 ", $single_proc->{time}, " 1 \n";
    foreach my $result (@results)
    {
        my $speedup = $single_proc->{time}/$result->{time};
        print $fh $result->{ncpus}, " ", $result->{time}, " " ,
           $speedup, "\n";
    }
    close($fh);
    foreach my $result (@results)
    {
        my $ncpus = $result->{ncpus};
        open($fh, ">$graph.$algo.balance.$ncpus.dat");
        my @bal = @{$result->{balance}};
        for (my $k=0;  $k < @bal; $k++)
        {
            print $fh "$k $bal[$k]\n";
        }
        close($fh);
    }
    foreach my $result (@results)
    {
        my $ncpus = $result->{ncpus};
        open($fh, ">$graph.$algo.load.$ncpus.dat");
        my %load = %{$result->{load}};
        for (my $k=0;  $k < @{$load{0}}; $k++)
        {
            print $fh "$k ";
            for (my $i=0; $i < $ncpus; $i++)
            {
                my @series = @{$load{$i}};
                print $fh "$series[$k] ";
            }
            print $fh "\n";
        }
        close($fh);
    }
}

# bigger pile of hacks... ec2 junk
foreach my $algo (@algorithms)
{
    my @results = ();
    for (my $i=2; $i <= 4; $i++)
    {
        my $fn="oleo/ec2.bfs.$algo.$i";
        open($fh, $fn) || die "$! opening $fn";
        push @results, parse_trace($fh, $algo);
        close($fh);
    }
    open($fh, ">oleo/ec2.$algo.time.dat");
    print $fh "1 ", $ec2_single_proc->{time}, " 1 \n";
    my $i=2;
    foreach my $result (@results)
    {
        my $speedup = $ec2_single_proc->{time}/$result->{time};
        print $fh $i, " ", $result->{time}, " " ,
           $speedup, "\n";
        $i++;
    }
    close($fh);
}

#another big pile..
$graph = "oleo/twitter_graph_sparse";
foreach my $algo ("rand","rr","pmetis")
{
    my @results = ();

    for (my $i=2; $i <= 4; $i++)
    {
        my $fh;
        my $fn = "$graph.trace.bfs.$algo.$i";
        open($fh, $fn) || die "$! opening $fn";
        push @results, parse_trace($fh, $algo);
        close($fh);
    }
    open($fh, ">$graph.$algo.time.dat");
    print $fh "1 ", $single_proc->{time}, " 1 \n";
    foreach my $result (@results)
    {
        my $speedup = $single_proc->{time}/$result->{time};
        print $fh $result->{ncpus}, " ", $result->{time}, " " ,
           $speedup, "\n";
    }
    close($fh);
}


system("cp oleo/*dat paper/charts/ && cd paper/charts && gnuplot plot_all.plt");

sub parse_trace
{
    my ($fh,$title) = @_;
    my $ncpus = 0;
    my %balance;
    my $time;
    my @bal;
    my %load;
    
    while(<$fh>)
    {
        if (/iter: (\d+) (\d+) (\d+)/)
        {
            my $iteration = $1; 
            my $cpu = $2; 
            my $nodes = $3; 

            $ncpus = max($cpu+1, $ncpus);
            $balance{$iteration}->{value} =
                max($balance{$iteration}->{value}, $nodes);
            $balance{$iteration}->{sum} += $nodes;
            $balance{$iteration}->{$cpu} = $nodes;
        }

        foreach my $iter (sort keys(%balance))
        {
            if ($balance{$iter}->{sum} == 0) {
                push @bal, 1.0;
            } else {
                push @bal, $balance{$iter}->{value} * $ncpus /
                       $balance{$iter}->{sum};
            }
            foreach my $cpu (keys (%{$balance{$iter}})) {
                push @{$load{$cpu}}, $balance{$iter}->{$cpu};
            }
        }

        if (/ran in (\d+):(\d+):([0-9.]*) s/)
        {
            $time = 3600 * $1 + 60 * $2 + $3;
        }
    }
    my $result = {
        title => $title,
        time => $time,
        balance => \@bal,
        load => \%load,
        ncpus => $ncpus
    };
    return $result;
}
