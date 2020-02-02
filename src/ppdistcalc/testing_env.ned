//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

import src.PPDistNode;

network TestingEnvironment
{
    @display("bgb=526,396");
    submodules:
        alice: PPDistNode {
            @display("p=105,259");
        }
        bob: PPDistNode {
            @display("p=302,98");
        }
    connections:
        alice.out --> {  delay = 100ms; } --> bob.in;
        alice.in <-- {  delay = 100ms; } <-- bob.out;
}
