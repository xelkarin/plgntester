#
# Created:  Fri 09 Feb 2018 06:27:45 PM PST
# Modified: Fri 09 Feb 2018 06:30:58 PM PST
#
# Copyright 2018 (C) Robert Gill
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

all:
	make -C src

clean:
	make -C src clean

.PHONY: all clean
