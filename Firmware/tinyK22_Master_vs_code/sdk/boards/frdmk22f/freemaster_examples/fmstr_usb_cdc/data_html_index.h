/*
 * Copyright (c) 2007-2015 Freescale Semiconductor, Inc.
 * Copyright 2018-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * FreeMASTER data file index.html
 */

FMSTR_DEMO_CONTENT char index_htm[]  =
"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">"
"<html><head>"
"<style>"
"body{background: #EAEAEA; font-family: Sans-serif; padding: 0px; margin: 0px;}"
"h1, h2, h3{color: #1D00A3;}"
"img{border: 0px;}"
"a{color: #0000BA; font-weight: bold; text-decoration: none;}"
"a:hover{text-decoration: underline;}"

".header{display: block; margin: 0px; background: #44627C; padding: 15px; border-bottom: 3px solid black}"
".header h1{color: white;}"
".body{padding: 10px;}"
".footer{border-top: 1px solid silver; padding: 10px; font-size: 8pt; width: 100%;}"

".files{display: table-cell; width: 100%; border: 1px; text-align: center;}"
".files li{display: block; float: left; background: linear-gradient(45deg, #7CB3FF, #ADD0FF); height: 200px; width: 200px; border: 1px solid gray; margin: 10px; padding: 5px; text-align: center; border-radius: 10px; box-shadow: 2px 2px 5px gray;}"
".files li img{display: block; width: 50px; border: 0px; margin: 25px auto 15px auto;}"
".files li:hover{position: relative; background: linear-gradient(45deg, #7CCDFF, #5B8FFF); top: 1px; left: 1px;}"
"</style>"
"</head><body>"

"<div class=\"header\">"
"<a href=\"http://www.nxp.com\" target=\"_BLANK\"><img src=\"//images/logo.png\" align=\"right\" alt=\"NXP logo should be visible if the image file was compiled in.\" /></a>"
"<h1>This file was retrieved from the board memory!</h1>"
"</div>"

"<div class=\"body\">"
"<p>This is a sample HTML file stored in target device memory. Such files can be accessed in the FreeMASTER browser window when using a special <b>fmstr:</b> protocol specifier. This new feature enables you to embed text files, images or scripts directly into the target device.</p>"
"<p>"
"As a standard HTML file, you can use references to other files stored in the target device. Here are few examples implemented by this demo:"

"<ul class=\"files\">"
"<li><a href=\"pcmaster:appctl:openprj:fmstr://demo.pmp.zip\"><img src=\"//images/fmstrlogo.png\" />Demo project</a> <br />General demo project fetched from the target memory.</li>"
"<li><a href=\"fmstr://readme.txt\"><img src=\"//images/file.png\" />/readme.txt</a><br />Plain-text file.</li>"
"<li><a href=\"fmstr://text_files/one.txt\"><img src=\"//images/file.png\" />/text_files/one.txt</a><br />Plain-text file located in a virtual folder.</li>"
"<li><a href=\"fmstr://text_files/two.txt\"><img src=\"//images/file.png\" />/text_files/two.txt</a><br />Plain-text file in the same virtual folder.</li>"
"<li><a href=\"fmstr://other/three.txt\"><img src=\"//images/file.png\" />/other/three.txt</a><br />Plain-text file in another virtual folder.</li>"

"</ul>"
"</p>"
"</div>"

"<br />"
"<br />"

"<div class=\"footer\">"
"&copy; Copyright 2018-2019 NXP"
"</div>"

"</body></html> ";
