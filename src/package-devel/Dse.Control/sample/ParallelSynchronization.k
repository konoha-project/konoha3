// You have to start DSE daemon in advance.
// This script works as following scenario.


/*
           --->D-Task2----
           |    @DSE1    |
           |             |
D-Task1----               --->D-Task3
 @DSE1     |             |     @DSE2
           |             |
           --->D-Task2---
                @DSE2
*/

import("Type.Json");
import("Syntax.GlobalVariable");
import("Dse.Control");

assert(DControl.start(8090)); // D-Control is work on a port 8090 (this port is not the port of DSE)

DTask dtask1 = new DTask("D-Task1.k"); // D-Task1
DTask dtask2 = new DTask("D-Task2.k"); // D-Task2
DTask dtask3 = new DTask("D-Task3.k"); // D-Task3

Dse dse1 = new Dse("0.0.0.0"); // think it as DSE1
dse1.appendDTask(dtask1);
Json[] result1 = dse1.dispatchDTasks(); // result of D-Task1 on DSE1
System.p("result of D-Task1 on DSE1")
System.p(result1[0]);

DseGroup dse12 = new DseGroup() // think it as DSE1 & DSE2
dse12.appendDse("0.0.0.0"); // think it as DSE1
dse12.appendDse("0.0.0.0"); // think it as DSE2
dse12.appendDTask(dtask2);
Json[] result2 = dse12.dispatchDTasks(); // result of D-Task2 on DSE1 & DSE2
System.p("result of D-Task2 on DSE1 & DSE2");
System.p(result2[0]);
System.p(result2[1]);

Dse dse2 = new Dse("0.0.0.0"); // think it as DSE2
dse2.appendDTask(dtask3);
Json[] result3 = dse2.dispatchDTasks();
System.p("result of D-Task3 on DSE2");
System.p(result3[0]);
