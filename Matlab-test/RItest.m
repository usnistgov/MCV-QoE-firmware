%RItest - test a radio interface board
%
% The RITEST script runs through a test sequence for the RadioInterface
% board. This can be done on a newly setup board to check that everything
% is setup correctly or on an older board to see if it brke. 
%To complete all parts of the test, you will need a multimeter and/or an
%oscilloscope and a wire to connect P6.0 to 3.3V. Alternately a variable
%bench supply can be used to determine the switching threshold. If all that
%is desired is to determine that the firmware was loaded correctly and able
%to communicate with MATLAB, simply vusually checking that the LEDs come on
%is sufficient.
 

%This software was developed by employees of the National Institute of
%Standards and Technology (NIST), an agency of the Federal Government.
%Pursuant to title 17 United States Code Section 105, works of NIST
%employees are not subject to copyright protection in the United States and
%are considered to be in the public domain. Permission to freely use, copy,
%modify, and distribute this software and its documentation without fee is
%hereby granted, provided that this notice and disclaimer of warranty
%appears in all copies.
%
%THE SOFTWARE IS PROVIDED 'AS IS' WITHOUT ANY WARRANTY OF ANY KIND, EITHER
%EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, ANY
%WARRANTY THAT THE SOFTWARE WILL CONFORM TO SPECIFICATIONS, ANY IMPLIED
%WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
%FREEDOM FROM INFRINGEMENT, AND ANY WARRANTY THAT THE DOCUMENTATION WILL
%CONFORM TO THE SOFTWARE, OR ANY WARRANTY THAT THE SOFTWARE WILL BE ERROR
%FREE. IN NO EVENT SHALL NIST BE LIABLE FOR ANY DAMAGES, INCLUDING, BUT NOT
%LIMITED TO, DIRECT, INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES, ARISING
%OUT OF, RESULTING FROM, OR IN ANY WAY CONNECTED WITH THIS SOFTWARE,
%WHETHER OR NOT BASED UPON WARRANTY, CONTRACT, TORT, OR OTHERWISE, WHETHER
%OR NOT INJURY WAS SUSTAINED BY PERSONS OR PROPERTY OR OTHERWISE, AND
%WHETHER OR NOT LOSS WAS SUSTAINED FROM, OR AROSE OUT OF THE RESULTS OF, OR
%USE OF, THE SOFTWARE OR SERVICES PROVIDED HEREUNDER.

fprintf(['Starting radioInterface test.\n'...
    'searching for radio interface...\n']);

%create new radioInterface
ri=radioInterface();

%if an interface is not found than there will be an error, if we get here
%then there was no error and the interface has connected successfully
fprintf('Successfully attached!\n');

fprintf(['Testing LEDs. LEDs will flash alternately 10 times.\n'...
         'Press enter to continue\n']); 
pause;

for k=1:10
    for ledNum=1:2
        ri.led(ledNum,true);
        pause(0.1);
        ri.led(ledNum,false);
    end
end

fprintf('LED test complete!\n');

%turn off PTT for good measure
ri.ptt(false);

fprintf(['Testing PTT output off.\n'...
         'P2.5 and P8.2 should read 0 V.\n'...
         'press enter when done.\n']);
pause;
%set push to talk output
ri.ptt(true);
%notify user of test
fprintf(['Testing PTT output off.\n'...
         'P2.5 should output a 3.3 V squarwave.\n'...
         'P8.2 should read 3.3 V.\n'...
         'press enter when done.\n']);
pause;
%turn off PTT
ri.ptt(false);
fprintf('PTT test complete\n');

fprintf(['Start signal test.\n'...
         'Press enter to continue...\n']);

pause;

%set the ptt with delay and start signal
ri.ptt_delay(0,'UseSignal',true);

fprintf(['Waiting for start signal. Apply between 0.7 V and 3.3 V to P6.0.\n'...
         'Test will timeout after about 60 seconds.\n']);

%loop for about 60 seconds
for k=1:(60/0.1)
    %get the wait state from radio interface
    state=ri.WaitState;
    
    %check wait state to see if PTT was triggered properly
    switch(state)
        case 'Idle'
            fprintf('Signal detected, success!\n');
            break;
        case 'Signal Wait'
            %still waiting for start signal
        case 'Delay'
            %Start signal detected, waiting for delay to expire
            %this should not happen in this case
            fprintf('Delay')
        otherwise
            %delete radio interface before giving error
            delete(ri)
            %unknown state
            error('Unknown radio interface wait state ''%s''',state)
    end
    %pause for a bit
    pause(0.1)
end

%turn off PTT and clear wait condition
ri.ptt(false);

%check for timeout
if(~strcmp(state,'Idle'))
    fprintf('Test timed out start signal not detected\n');
end

delete(ri);