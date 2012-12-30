<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8"/>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link href="/bt/css/bootstrap.css" rel="stylesheet">
    <link rel="stylesheet" href="/bt/css/font-awesome.css">
    <link rel="stylesheet" href="/bt/css/bootstrap-arrows.css">
    <style type="text/css">
      body {
        padding-top: 60px;
        padding-bottom: 40px;
      }
      .sidebar-nav {
        padding: 9px 0;
      }
    </style>
    <link href="/bt/css/bootstrap-responsive.css" rel="stylesheet">

    <!-- HTML5 shim, for IE6-8 support of HTML5 elements -->
    <!--[if lt IE 9]>
      <script src="http://html5shim.googlecode.com/svn/trunk/html5.js"></script>
    <![endif]-->
<title>Repetier-Server</title>
</head>
<body>
	
    <div class="navbar navbar-inverse navbar-fixed-top">
      <div class="navbar-inner">
        <div class="container">
          <a class="btn btn-navbar" data-toggle="collapse" data-target=".nav-collapse">
            <span class="icon-bar"></span>
            <span class="icon-bar"></span>
            <span class="icon-bar"></span>
          </a>
          <a class="brand" href="http://www.repetier.com/server">Repetier-Server {{version}}</a>
          <div class="nav-collapse collapse">
            <ul class="nav">
              <li><a href="/index.php"><?php _("Home") ?></a></li>
              <li class="active"><a href="/about.php"><?php _("About")?></a></li>
              <li><a href="/documentation.php"><?php _("Documentation") ?></a></li>
            </ul>
          </div><!--/.nav-collapse -->
        </div>
      </div>
    </div>
    <div class="container">
    <div class="row">  
	<div class="span12">  
		<ul class="breadcrumb">  
  		<li>  
    		<a href="index.php"><i class="icon-home"></i> <?php _("Printer overview") ?></a> <span class="divider"> &gt; </span>  
  		</li>  
  		<li class="active"><?php _("About")?></span></li>  
		</ul>  
	</div>  
</div>  
<div class="row"><span class="span12">
<h1>Repetier-Server licence</h1>
<p>My part of the work is distributed under the Apache V2 licence.
The sources contain software from 3rd parties with different licences.</p>

<p>Licensed under the Apache License, Version 2.0 (the "License"); you may not use this work
except in compliance with the License. You may obtain a copy of the License in the
 LICENSE file, or at:</p>

<a href="http://www.apache.org/licenses/LICENSE-2.0" target="_blank">http://www.apache.org/licenses/LICENSE-2.0</a>

<p>Unless required by applicable law or agreed to in writing, software distributed under the
License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
either express or implied. See the License for the specific language governing
permissions and limitations under the License.</p>

<h2>3rd party software uses with the server</h2>

<h3>moFileReader - A simple .mo-File-Reader</h3>
<p>Copyright (C) 2009 Domenico Gentner (scorcher24@gmail.com)
All rights reserved.    </p>

<p>Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:</p>
<ol>
<li> Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.</li>
<li> Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.</li>

<li> The names of its contributors may not be used to endorse or promote 
     products derived from this software without specific prior written 
     permission.</li>
 </ol>
<p>  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
</p>

<h3>JSON Spirit</h3>

Homepage: <a href="http://www.codeproject.com/Articles/20027/JSON-Spirit-A-C-JSON-Parser-Generator-Implemented" target="_blank">http://www.codeproject.com/Articles/20027/JSON-Spirit-A-C-JSON-Parser-Generator-Implemented</a>
<p>Copyright John W. Wilkinson 2007 - 2011</p>
<p>Distributed under the MIT License, see accompanying file LICENSE.txt</p>

<h3>Mongoose webserver</h3>

<p>Copyright (c) 2004-2010 Sergey Lyubka</p>

<p>Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
</p><p>
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
</p><p>
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
</p>
<h3>libconfig - A library for processing structured configuration files</h3>
   <p>Copyright (C) 2005-2010  Mark A Lindner</p>

   <p>This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.
</p><p>
   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
</p>
  <p> You should have received a copy of the GNU Library General Public
   License along with this library; if not, see
   <a href="http://www.gnu.org/licenses" target="_blank">http://www.gnu.org/licenses</a>.
   </p>
   
<h3>Twitter Bootstrap</h3>

<p>Homepage: <a href="https://github.com/twitter/bootstrap" target="_blank">https://github.com/twitter/bootstrap</a></p>

<p>Copyright 2012 Twitter, Inc.</p>

<p>Licensed under the Apache License, Version 2.0 (the "License"); you may not use this work
except in compliance with the License. You may obtain a copy of the License in the
 LICENSE file, or at:</p>

<a href="http://www.apache.org/licenses/LICENSE-2.0" target="_blank">http://www.apache.org/licenses/LICENSE-2.0</a>

<p>Unless required by applicable law or agreed to in writing, software distributed under the
License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
either express or implied. See the License for the specific language governing
permissions and limitations under the License.</p>

<h3>jQuery</h3>

<p>Homepage: <a href="http://jquery.com/" target="_blank">http://jquery.com/</a></p>
<p>Licence: MIT Licence</p>

<h3>jQuery form</h3>

Homepage: <a href="http://malsup.com/jquery/form/" target="_blank">http://malsup.com/jquery/form/</a>
<p>The jQuery Form plugin is dual licensed under the MIT and GPL licenses:</p>
<ul>
<li><a href="http://malsup.github.com/mit-license.txt">MIT</a></li>
<li><a href="http://malsup.github.com/gpl-license-v2.txt">GPL</a></li>
</ul>
<p>You may use either license.  The MIT License is recommended for most projects because
 it is simple and easy to understand and it places almost no restrictions on what you can
  do with the plugin.</p>
<p>If the GPL suits your project better you are also free to use the plugin under that license.</p>
<p>You don't have to do anything special to choose one license or the other and you don't
 have to notify anyone which license you are using. You are free to use the jQuery Form
  Plugin in commercial projects as long as the copyright header is left intact.</p>

<h3>Bootbox.js</h3>

<p>Copyright (C) 2011-2012 by Nick Payne <nick@kurai.co.uk></p>
Homepage: <a href="http://bootboxjs.com/index.html" target="_blank">http://bootboxjs.com/index.html</a>
<p>MIT Licence <a href="http://bootboxjs.com/license.txt" target="_blank">http://bootboxjs.com/license.txt</a></p>

</span></div>
</div> <!-- /container -->

<!-- Placed at the end of the document so the pages load faster -->
<script src="/jquery/jquery.min.js"></script>
<script src="/jquery/jquery.form.js"></script>
<script src="/bt/bootstrap.min.js"></script>
</body></html>
