#! /bin/bash

########################################################################
#                                                                      #
# An example shell script to read values from GoQat, and pass back     #
# some parameters.  Call this script from DemoTasks.txt.               #
#                                                                      #
########################################################################



########################################################################
#                                                                      #
# Keep the following section in your scripts.  This section obtains    #
# values from GoQat and assigns them to named variables.  (Of course,  #
# you can change these variable names if you wish).                    #
#                                                                      #
########################################################################

a=0;
args=("$@");

sync=${args[a++]};                 # 1 if synchronous, 0 if asynchronous
results_file=${args[a++]};         # If synchronous, name of file read by GoQat when script finishes
done_file=${args[a++]};            # If synchronous, script must create this file to indicate it has finished
num_task_params=${args[a++]};      # Number of task parameters (%0, %1... etc)

for ((i=0;i<$num_task_params;i++)); do
  task_params[i]=${args[a++]};     # Array containing task parameters
done

Expose_image_file=${args[a++]};    # Name of most recently saved CCD FITS file
Expose_exposure_type=${args[a++]}; # Exposure type
Expose_filter=${args[a++]};        # Exposure filter
Expose_seconds=${args[a++]};       # Exposure length
Expose_H1=${args[a++]};            # Exposure H1 coordinate
Expose_V1=${args[a++]};            # Exposure V1 coordinate
Expose_H2=${args[a++]};            # Exposure H2 coordinate
Expose_V2=${args[a++]};            # Exposure V2 coordinate
Expose_Hbin=${args[a++]};          # H-binning
Expose_Vbin=${args[a++]};          # V-binning
Expose_degC=${args[a++]};          # Chip temperature

Coords_RA=${args[a++]};            # Current RA position of telescope
Coords_Dec=${args[a++]};           # Current Dec position of telescope

Focus_max_travel=${args[a++]};     # Maximum travel of focuser 
Focus_cur_pos=${args[a++]};        # Current focuser postion
Focus_temp=${args[a++]};           # Current focuser temperature

########################################################################
#                                                                      #
#                 END OF VARIABLE ASSIGNMENT SECTION                   #
#                                                                      #
########################################################################



########################################################################
#                                                                      #
# Execute your own commands below.  These can be anything that you can #
# do in a shell script.                                                #
#                                                                      #
########################################################################

# Here's an example command (subtract the value of parameter 0 from the 
# current CCD temperature and assign the value to a new variable).  Note
# the use of bc to do the maths since the values in the equation might
# be floating-point variables:

value=`echo "$Expose_degC - ${task_params[0]}" | bc`;

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
if [ $sync -eq 1 ]; then
#                                                                      #
########################################################################

# Here is an example of passing a parameter back to GoQat.
# The variable '$value' defined above is passed back as parameter 9.
# Use '>>' rather than '>' for any subsequent lines, so that they are 
# appended to the existing file, rather than overwriting it.

  echo "%9 $value" > $results_file;

# If the new chip temperature '$value' is less than -5C, then exit the
# 'While' loop in the task list.  Do this by passing back a value
# of '0' for parameter 1.  (Otherwise, GoQat continues to use the
# existing value).

  if [ `echo "$value < -5" | bc` -eq 1 ]; then 
     echo "%1 0" >> $results_file;
  fi
  
# Write a comment that will be displayed by GoQat in the log window when
# the script has finished.  Note the hash '#' at the beginning of the
# line, and the use of '>>' rather than '>'. 

  echo "#The current chip temperature is $Expose_degC" >> $results_file;
  echo "#A value of ${task_params[0]} has been subtracted from this" >> $results_file;
  echo "#and returned in parameter %9" >> $results_file;

########################################################################
#                                                                      #
# Finally, create the file that tells GoQat the script has finished.   #
#                                                                      #
  touch $done_file;
fi
#                                                                      #
########################################################################

exit 0;
