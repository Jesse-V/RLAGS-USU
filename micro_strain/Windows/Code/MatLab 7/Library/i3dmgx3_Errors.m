function ErrorMessages = i3dmgx3_Errors
%Creates the Command array in the MatLab workspace
ErrorMessages = {
    'Could not open serial link on specified port';
    'Could not set communication parameters on specified port';
    'Could not set timeouts on specified port';
    'Could not write to device on specified port';
    'Output does not match command';
    'Could not read device';
    'Could not clear buffer';
    'Incorrect checksum'};