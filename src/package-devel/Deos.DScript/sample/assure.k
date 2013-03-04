Import("Deos.DScript");

@DSE1 assure reboot must work {
	System.p("sudo reboot -h now");
}

argue reboot must work {
	strategy pre {
		@DSE2 assure web server must be alive before the fact {
			System.p("telnet @IP @PORT");
		}
	}
	strategy post {
		@DSE2 assure web server must be alive after the fact {
			System.p("telnet @IP @PORT");
		}
	}
}

System.p(@[reboot must work]);
