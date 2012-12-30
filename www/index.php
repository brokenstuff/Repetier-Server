<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8"/>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link href="/bt/css/bootstrap.css" rel="stylesheet">
    <link rel="stylesheet" href="/bt/css/font-awesome.css">
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
              <li class="active"><a href="/index.php"><?php _("Home") ?></a></li>
              <li><a href="/about.php"><?php _("About")?></a></li>
              <li><a href="/documentation.php"><?php _("Documentation") ?></a></li>
            </ul>
          </div><!--/.nav-collapse -->
        </div>
      </div>
    </div>

    <div class="container">
    <div id="msgcontainer"></div>
<div class="row">  
<div class="span12">  
<ul class="breadcrumb">  
 <li>  
    <i class="icon-home"></i> <?php _("Printer overview") ?></a>  
  </li>  
</ul>  
</div>  
</div>  

<div class="row"><div class="span12">
      <table class="table table-striped table-hover table-bordered">  
        <thead>  
          <tr>  
            <th><i class="icon-print"></i> <?php _("Printer")?></th>  
            <th><i class="icon-off"></i> <?php _("Online")?></th>  
            <th><i class="icon-tasks"></i> <?php _("Job")?></th>  
            <th><i class="icon-cog"></i> <?php _("Action")?></th>
          </tr>  
        </thead>  
        <tbody id="printerlist">  
        </tbody>  
      </table> 
</div></div>
    </div> <!-- /container -->

<!-- Placed at the end of the document so the pages load faster -->
<script src="/jquery/jquery.min.js"></script>
<script src="/bt/bootstrap.min.js"></script>
<script src="/jquery/bootbox.min.js"></script>

<script type="text/javascript">
function htmlescape(t) {
	return t.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
}
function removeMsg(lnk) {
	$.getJSON(lnk,function(data) {updateMsg(data);});
} 
function updateMsg(data) {
	if(typeof data.messages === 'undefined') return;
	ids = new Array();
	$.each(data.messages,function(key,val) {
	  ids.push(""+val.id);
		if($('#msg'+val.id).length==0) {
		  s = '<div class="alert alert-block alert-info msg" id="msg'+val.id+'">';
      s += '<a class="close" onclick="removeMsg(\''+val.link+'\')" href="#">X</a>';
     	s += '<h4 class="alert-heading">Info</h4>';
      s += htmlescape(val.msg);
			s += '</div>';
	    $('#msgcontainer').before(s);
		}
	});
	$(".msg").each(function(idx,elem) {
	   id = $(elem).prop('id').substr(3);
	   if(ids.indexOf(id)<0) $(elem).remove(); 
	});
}
function setActive(prt,state) {
  if(!state) {
	bootbox.dialog("<?php _("Do you really want to disconnect the printer?")?>", [{
    "label" : "<?php _("Yes")?>",
    "class" : "btn-danger",
    "callback": function() {
		$.getJSON('/printer/pconfig/'+prt+'?a=active&mode='+(state ? 1 : 0),
	  	function(data) {updatePrinterList(data);});
    }
	}, {
    "label" : "<?php _("No")?>","class":"btn"
	}]);
  } else {
		$.getJSON('/printer/pconfig/'+prt+'?a=active&mode='+(state ? 1 : 0),
	  	function(data) {updatePrinterList(data);});
	}
}
function updatePrinterList(data) {
  s = "";
  $.each(data.data, function(key,val) {
    s+="<tr><td>"+val.name+"</td><td>";
    if(!val.active) s+= '<span class="label"><?php _("Deactivated") ?></span>';
    else if(val.online) s+= '<span class="label label-success"><?php _("Online") ?></span>';
    else s+='<span class="label label-important"><?php _("Offline") ?></span>';
    s+="</td><td>"+val.job+"</td><td>";
    s+='<a href="printer.php?pn='+val.slug+'" class="btn"><i class="icon-signin"></i> <?php _("Manage") ?></a> &nbsp; ';
    if(val.active)
      s+='<button onclick="setActive(\''+val.slug+'\',false)" class="btn btn-danger"><i class="icon-off"></i> <?php _("Deactivate") ?></button>';
    else
      s+='<button onclick="setActive(\''+val.slug+'\',true)" class="btn btn-success"><i class="icon-off"></i> <?php _("Activate") ?></button>';
    s+="</td></tr>";
  });
  $('#printerlist').html(s);  
  updateMsg(data);
}
function refreshPrinterList() {
	$.getJSON('/printer/list', function(data) {
		updatePrinterList(data);
  	setTimeout('refreshPrinterList()', 10000);
	});
};
$(document).ready(function() {
   refreshPrinterList();
 });
</script>
</body>
</html>
