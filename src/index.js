/* Copyright 2023 MarcosHCK
 * This file is part of WebServer.
 *
 * WebServer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * WebServer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with WebServer. If not, see <http://www.gnu.org/licenses/>.
 */
'use strict';

var gTable, gOrderBy, gTBody, gRows, gUI_showHidden;

function compareRows (rowA, rowB)
{
  var a_ = rowA.cells [gOrderBy].getAttribute ("sortable-data") || "", intA = +a_;
  var b_ = rowB.cells [gOrderBy].getAttribute ("sortable-data") || "", intB = +b_;
  var a = (a_ == intA) ? intA : a_.toLowerCase ();
  var b = (b_ == intB) ? intB : b_.toLowerCase ();
return (a < b) ? -1 : ((a == b) ? 0 : 1);
}

function orderBy (column)
{
  if (!gRows)
    gRows = Array.from (gTBody.rows);

  var order;

  if (gOrderBy == column)
    {
      order = gTable.getAttribute ("order") == "asc" ? "desc" : "asc";
    }
  else
    {
      order = "asc";
      gOrderBy = column;
      gTable.setAttribute ("order-by", column);
      gRows.sort (compareRows);
    }

  gTable.removeChild (gTBody);
  gTable.setAttribute ("order", order);

  if (order == "asc")
    for (var i = 0; i < gRows.length; i++)
      gTBody.appendChild (gRows[i]);
  else
    for (var i = gRows.length - 1; i >= 0; i--)
      gTBody.appendChild (gRows[i]);

  gTable.appendChild (gTBody);
}

function updateHidden ()
{
  gTable.className = gUI_showHidden.getElementsByTagName ("input") [0].checked ? "" : "remove-hidden";
}

document.addEventListener ("DOMContentLoaded",
  function ()
    {
      gTable = document.getElementById ("UI_fileTable");
      gTBody = document.getElementById ("UI_fileList");
      gUI_showHidden = document.getElementById ("UI_showHidden");

      var headCells = gTable.tHead.rows [0].cells;
      var hiddenObjects = false;

      gUI_showHidden.getElementsByTagName ("input") [0].checked = false

      function rowAction (i)
        {
          return function (event)
            {
              event.preventDefault ();
              orderBy (i);
            }
        }

      for (var i = headCells.length - 1; i >= 0; i--)
        {
          var anchor = document.createElement ("a");
          anchor.href = "";
          anchor.appendChild (headCells [i].firstChild);
          headCells [i].appendChild (anchor);
          headCells [i].addEventListener ("click", rowAction (i), true);
        }

      if (gUI_showHidden)
        {
          gRows = Array.from (gTBody.rows);
          hiddenObjects = gRows.some (row => row.className == "hidden-object");
        }

      gTable.setAttribute ("order", "");

      if (hiddenObjects)
        {
          gUI_showHidden.style.display = "block";
          updateHidden ();
        }

      orderBy (0);
    }, "false");
