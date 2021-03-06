#please dont edit this file...
package PCVS::Validate::MPC;
use strict;
use warnings;
use Exporter;
use Data::Dumper;
use vars qw(@ISA @EXPORT);

@ISA = 'Exporter';
@EXPORT = qw();

my $gconf;
my $max_cores;
my $max_nodes;

sub runtime_init
{
	(my $self, $gconf) = @_;
	$max_cores = $gconf->{cluster}{max_cores_per_node};
	$max_nodes = $gconf->{cluster}{max_nodes};
}

sub runtime_fini
{
}

sub get_ifdef
{
	my ($arg, $k, @c)= @_;
	foreach (0..$#c)
	{
		return $c[$_] if($k->[$_] eq $arg);
	}

	return undef;
}

sub runtime_valid
{
	my ($self, $keys, @args) = @_;
	my ($node, $proc, $mpi, $omp, $core, $net, $sched) = ();

	$node  = get_ifdef('n_node',$keys,@args);
	$proc  = get_ifdef('n_proc',$keys,@args);
	$mpi   = get_ifdef('n_mpi' ,$keys,@args); 
	$omp   = get_ifdef('n_omp' ,$keys,@args);
	$core  = get_ifdef('n_core',$keys,@args);
	$net   = get_ifdef('net' ,$keys,@args); 
	$sched = get_ifdef('sched',$keys,@args);

	# some auto-detections, because MPC is resilient
	# Order matters
	$node = 1 if (!defined $node);
	if(!defined $mpi)
	{ EXIT_MPI:{
		($mpi = $proc and last EXIT_MPI) if(defined $proc);
		($mpi = $node and last EXIT_MPI) if(defined $node);
		$mpi = 1;
	}}
	if(!defined $proc)
	{ EXIT_PROC:{
		($proc = $mpi and last EXIT_PROC) if(defined $mpi);
		($proc = $node and last EXIT_PROC) if(defined $node);
		$proc = 1;
	}}


	# please be sure to check if the iterator exist because the system emits
	# an undef when the iterator won't be unfolded for the current configuration
	return 0 if(defined $node and defined $max_nodes and $node > $max_nodes); 
	return 0 if(defined $node and defined $proc      and $node > $proc); 
	return 0 if(defined $proc and !defined $mpi);
	return 0 if(defined $proc and defined $mpi       and $proc > $mpi); 
	return 0 if(defined $proc and defined $mpi       and $proc > $mpi); 
	return 0 if(defined $proc and defined $core and defined $node and $core > ($max_cores * $node) / $proc and $core ne 1);
	return 0 if(defined $node and defined $sched and $node > 1 and $net eq "shmem");
	
	#print "Keep N=".(defined $node ? $node : "X").
		  #" p=".(defined $proc ? $proc : "X").
		  #" t=".(defined $mpi ? $mpi : "X").
		  #" o=".(defined $omp ? $omp : "X").
		  #" c=".(defined $core ? $core : "X"). 
		  #" n=".(defined $net ? $net : "X").
		  #" m=".(defined $sched ? $sched : "X")."\n";	
	return 1;
}

1;

