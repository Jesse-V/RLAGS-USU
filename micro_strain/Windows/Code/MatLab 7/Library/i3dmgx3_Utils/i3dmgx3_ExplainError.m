function i3dmgx3_ExplainError(Error)
%Display an appropriate error message when an error occurs
%
%Arguments: Error - Error number

ErrorText = i3dmgx3_Errors;
fprintf(['\nError: ' char(ErrorText(Error)) '\n']);