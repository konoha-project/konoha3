/****************************************************************************
 * KONOHA COPYRIGHT, LICENSE NOTICE, AND DISCRIMER
 *
 * Copyright (c) 2006-2011, Kimio Kuramitsu <kimio at ynu.ac.jp>
 *           (c) 2008-      Konoha Team konohaken@googlegroups.com
 * All rights reserved.
 *
 * You may choose one of the following two licenses when you use konoha.
 * If you want to use the latter license, please contact us.
 *
 * (1) GNU General Public License 3.0 (with KONOHA_UNDER_LGPL3)
 * (2) Konoha Non-Disclosure License 1.0
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

include "lib:gluelink";

using konoha.posix.*; // FILEクラスのインポート及び、SIGNAL番号再利用

@Native class Subproc;

/* Subproc */
@Native Subproc Subproc.new(String cmd, boolean closefds);
@Native boolean Subproc.bg();
@Throwable @Native int Subproc.fg(); 
@Throwable @Native String Subproc.exec(String data);
@Throwable @Native String[] Subproc.communicate(String input);
@Native int Subproc.poll();
@Native int Subproc.wait();
@Native boolean Subproc.sendSignal(int signal);
@Native boolean Subproc.terminate();
@Native boolean Subproc.kill(void);
@Native boolean Subproc.stop(void);
@Native boolean Subproc.start(); 
@Throwable @Native boolean Subproc.restart(); 

@Restricted @Native boolean Subproc.enableShellmode(boolean isShellmode);
@Native boolean Subproc.setEnv(Map env);
@Native boolean Subproc.setCwd(String cwd);
// @Native boolean Subproc.setBufieldsize(int size);
@Native boolean Subproc.setFileIN(File in);
@Native boolean Subproc.setFileOUT(File out);
@Native boolean Subproc.setFileERR(File err);
@Native boolean Subproc.setTimeout(int milisec);
@Native File Subproc.getIN();
@Native File Subproc.getOUT();
@Native File Subproc.getERR();
@Native int Subproc.getPid();
@Native int Subproc.getTimeout(); 
@Native boolean Subproc.enablePipemodeIN(Boolean isPipemode);
@Native boolean Subproc.enablePipemodeOUT(Boolean isPipemode);
@Native boolean Subproc.enablePipemodeERR(Boolean isPipemode);
@Native boolean Subproc.enableStandardIN(Boolean isStandard);
@Native boolean Subproc.enableStandardOUT(Boolean isStandard);
@Native boolean Subproc.enableStandardERR(Boolean isStandard);
@Native boolean Subproc.enableERR2StdOUT(Boolean isStdout);
@Native boolean Subproc.isShellmode();
@Native boolean Subproc.isPipemodeIN();
@Native boolean Subproc.isPipemodeOUT();
@Native boolean Subproc.isPipemodeERR();
@Native boolean Subproc.isStandardIN();
@Native boolean Subproc.isStandardOUT();
@Native boolean Subproc.isStandardERR();
@Native boolean Subproc.isERR2StdOUT();
@Native String[] Subproc.argSplit(String args);
