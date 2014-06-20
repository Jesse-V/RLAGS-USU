#! /usr/bin/perl -w

########################################################################
#                                                                      #
# An example Perl script to read values from GoQat, and pass back some #
# parameters.  Call this script from DemoTasks.txt.                    #
#                                                                      #
########################################################################



########################################################################
#                                                                      #
# Keep the following section in your scripts.  This section obtains    #
# values from GoQat and assigns them to named variables.  (Of course,  #
# you can change these variable names if you wish).                    #
#                                                                      #
########################################################################

my $a = 0;

my $sync = $ARGV[$a++];                 # 1 if synchronous, 0 if asynchronous
my $results_file = $ARGV[$a++];         # If synchronous, name of file read by GoQat when script finishes
my $done_file = $ARGV[$a++];            # If synchronous, script must create this file to indicate it has finished
my $num_task_params = $ARGV[$a++];      # Number of task parameters (%0, %1... etc)

my @task_params;
for (my $i = 0; $i < $num_task_params; $i++) {
  push (@task_params, $ARGV[$a++]);     # Array containing task parameters
}

my $Expose_image_file = $ARGV[$a++];    # Name of most recently saved CCD FITS file
my $Expose_exposure_type = $ARGV[$a++]; # Exposure type
my $Expose_filter = $ARGV[$a++];        # Exposure filter
my $Expose_seconds = $ARGV[$a++];       # Exposure length
my $Expose_H1 = $ARGV[$a++];            # Exposure H1 coordinate
my $Expose_V1 = $ARGV[$a++];            # Exposure V1 coordinate
my $Expose_H2 = $ARGV[$a++];            # Exposure H2 coordinate
my $Expose_V2 = $ARGV[$a++];            # Exposure V2 coordinate
my $Expose_Hbin = $ARGV[$a++];          # H-binning
my $Expose_Vbin = $ARGV[$a++];          # V-binning
my $Expose_degC = $ARGV[$a++];          # Chip temperature

my $Coords_RA = $ARGV[$a++];            # Current RA position of telescope
my $Coords_Dec = $ARGV[$a++];           # Current Dec position of telescope

my $Focus_max_travel = $ARGV[$a++];     # Maximum travel of focuser 
my $Focus_cur_pos = $ARGV[$a++];        # Current focuser postion
my $Focus_temp = $ARGV[$a++];           # Current focuser temperature

########################################################################
#                                                                      #
#                 END OF VARIABLE ASSIGNMENT SECTION                   #
#                                                                      #
########################################################################



########################################################################
#                                                                      #
# Execute your own commands below.  These can be anything that you can #
# do in a Perl script.                                                 #
#                                                                      #
########################################################################

# Here's an example command (subtract the value of parameter 0 from the 
# current CCD temperature and assign the value to a new variable):

my $value = $Expose_degC - $task_params[0];

########################################################################
#                                                                      #
#                     END OF YOUR OWN COMMANDS                         #            
#                                                                      #
########################################################################



########################################################################
#                                                                      #
# If the script is being run synchronously, you may pass the values of #
# the task parameters back to GoQat.  Do that below.                   #
#                                                                      #
if ($sync) {
#                                                                      #
########################################################################

# Here is an example of passing a parameter back to GoQat.

  open (RESULTS, '>', $results_file) or die "Can't open $results_file";
  
# The variable '$value' defined above is passed back as parameter 9.
# Note the line must be terminated by the new-line symbol '\n'.

  print (RESULTS "%9 $value\n");
  
# If the new chip temperature '$value' is less than -5C, then exit the
# 'While' loop in the task list.  Do this by passing back a value
# of '0' for parameter 1.  (Otherwise, GoQat continues to use the
# existing value).

  if ($value < -5) {print (RESULTS "%1 0\n");}
  
# Write a comment that will be displayed by GoQat in the log window when
# the script has finished.  Note the hash '#' at the beginning of the
# line and the new-line symbol '\n' at the end.

  print (RESULTS "#The current chip temperature is $Expose_degC\n");
  print (RESULTS "#A value of $task_params[0] has been subtracted from this\n");
  print (RESULTS "#and returned in parameter %9\n");

  close (RESULTS);

########################################################################
#                                                                      #
# Finally, create the file that tells GoQat the script has finished.   #
#                                                                      #
  open (DONE, '>', $done_file) or die "Can't open $done_file";
  close (DONE);
}
#                                                                      #
########################################################################

exit 0;
