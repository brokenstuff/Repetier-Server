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
          <a class="brand" href="http://www.repetier.com/server">Repetier-Server</a>
          <div class="nav-collapse collapse">
            <ul class="nav">
              <li class="active"><a href="#">Home</a></li>
              <li><a href="/about.php">About</a></li>
              <li><a href="/documentation.php">Documentation</a></li>
            </ul>
          </div><!--/.nav-collapse -->
        </div>
      </div>
    </div>

    <div class="container">
	<div id="dialog-uploadjob" class="modal hide fade in" style="display: none; ">
<div class="modal-header">  
<a class="close" data-dismiss="modal">×</a>  
<h3><?php _("Upload Job")?> </h3>  
</div>  
<div class="modal-body">  
<form enctype="multipart/form-data" action="/printer/job/{{slug}}" id="formuploadjob">
<input type="hidden" name="a" value="upload"/>
<label><?php _("Job Name") ?></label>
<input type="text" name="name" placeholder="Optional name"/>  
<label><?php _("G-Code File") ?></label>
<input type="file" name="file"/>
<label class="checkbox">
  <input type="checkbox" name="autostart" value="1" checked="checked">
  <?php _("Start job after upload if possible") ?>
</label></form>            
</div>  
<div class="modal-footer">  
<button id="startjobupload" class="btn btn-success" data-loading-text="<?php _("Uploading ...") ?>"><?php _("Start upload") ?></button>  
<button class="btn" data-dismiss="modal"><?php _("Close") ?></button>  
</div>  
</div>


<div class="row">  
<div class="span12">  
<ul class="breadcrumb">  
  <li>  
    <a href="index.php"><i class="icon-home"></i> <?php _("Printer overview") ?></a> <span class="divider"> &gt; </span>  
  </li>  
  <li class="active">{{printerName}} <span id="poffline" class="btn btn-danger btn-mini" style="display:none"><?php _("Offline")?></span></li>  
</ul>  
</div>  
</div>  
<div class="row"><div class="span12">
<div class="tabbable"> <!-- Only required for left/right tabs -->
  <ul class="nav nav-tabs">
    <li class="active"><a href="#tab1" data-toggle="tab"><?php _("Jobs")?></a></li>
    <li><a href="#tab2" data-toggle="tab"><?php _("Models")?></a></li>
    <li><a href="#tab3" data-toggle="tab"><?php _("Control")?></a></li>
  </ul>
  <div class="tab-content">
    {{! ============== Job panel ================ }}
    <div class="tab-pane active" id="tab1">
	  <div class="row"><span class="span12"><a data-toggle="modal" href="#dialog-uploadjob" class="btn btn-primary btn-large"><i class="icon-plus-sign"></i> <?php _("Upload job")?></a><br/><br/>
      <table class="table table-striped table-hover table-bordered">  
        <thead>  
          <tr>  
            <th><i class="icon-tasks"></i> <?php _("Job")?></th>  
            <th><i class="icon-save"></i> <?php _("Size")?></th>  
            <th><i class="icon-off"></i> <?php _("Status")?></th>  
            <th><i class="icon-cog"></i> <?php _("Action")?></th>
          </tr>  
        </thead>  
        <tbody id="joblist">  
        </tbody>  
      </table> 
      </span></div>
      </div>
    {{! ============== Model panel ================ }}
    <div class="tab-pane" id="tab2">
      <p>Howdy, I'm in Section 2.</p>
    </div>
    {{! ============== Control panel ================ }}
    <div class="tab-pane" id="tab3">
      <div class="row">
      <div class="span12">
        <form class="navbar-form pull-left" id="formcmd">
      		<div class="input-append">
  	    	<input type="text" class="span6 inputonline" id="command" placeholder="<?php _("Type your G-Code Command")?>"/>
        	<button id="sendcmd" class="btn inputonline"><?php _("Send")?></button>        
    			</div>
      	</form></div>
      </div>
      <div class="row">
        <div class="span2"><?php _("Position") ?></div>
        <div class="span2" id="posx">X: </div>
        <div class="span2" id="posy">Y: </div>
        <div class="span2" id="posz">Z: </div>
      	<button type="button" class="btn btn-primary span1" data-toggle="collapse" data-target="#movepos"><i class="icon-home"></i></button> 
      	<button type="button" class="btn btn-primary span3" data-toggle="collapse" data-target="#movepos"><i class="icon-move"></i> <?php _("Move head") ?></button> 
      </div>
			<div id="movepos" class="collapse">
				<div class="row" style="margin-top:8px">
					<button class="span1 btn-success inputonline"><i class="icon-home"></i> X</button>
					<button class="span1 btn-success inputonline"><i class="icon-arrow-left"></i> -50</button>
					<button class="span1 btn-success inputonline"><i class="icon-arrow-left"></i> -10</button>
					<button class="span1 btn-success inputonline"><i class="icon-arrow-left"></i> -1</button>
					<button class="span1 btn-success inputonline"><i class="icon-arrow-left"></i> -0.1</button>
					<button class="span1 btn-success inputonline"><i class="icon-arrow-right"></i> 0.1</button>
					<button class="span1 btn-success inputonline"><i class="icon-arrow-right"></i> 1</button>
					<button class="span1 btn-success inputonline"><i class="icon-arrow-right"></i> 10</button>
					<button class="span1 btn-success inputonline"><i class="icon-arrow-right"></i> 50</button>
				</div>
				<div class="row" style="margin-top:8px">
					<button class="span1 btn-info inputonline"><i class="icon-home"></i> Y</button>
					<button class="span1 btn-info inputonline"><i class="icon-arrow-down"></i> -50</button>
					<button class="span1 btn-info inputonline"><i class="icon-arrow-down"></i> -10</button>
					<button class="span1 btn-info inputonline"><i class="icon-arrow-down"></i> -1</button>
					<button class="span1 btn-info inputonline"><i class="icon-arrow-down"></i> -0.1</button>
					<button class="span1 btn-info inputonline"><i class="icon-arrow-up"></i> 0.1</button>
					<button class="span1 btn-info inputonline"><i class="icon-arrow-up"></i> 1</button>
					<button class="span1 btn-info inputonline"><i class="icon-arrow-up"></i> 10</button>
					<button class="span1 btn-info inputonline"><i class="icon-arrow-up"></i> 50</button>
				</div>
				<div class="row" style="margin-top:8px">
					<button class="span1 btn-info inputonline"><i class="icon-home"></i> Z</button>
					<button class="span1 btn-info inputonline"><i class="icon-arrow-down"></i> -10</button>
					<button class="span1 btn-info inputonline"><i class="icon-arrow-down"></i> -1</button>
					<button class="span1 btn-info inputonline"><i class="icon-arrow-down"></i> -0.1</button>
					<button class="span1 btn-info inputonline"><i class="icon-arrow-down"></i> -0.01</button>
					<button class="span1 btn-info inputonline"><i class="icon-arrow-up"></i> 0.01</button>
					<button class="span1 btn-info inputonline"><i class="icon-arrow-up"></i> 0.1</button>
					<button class="span1 btn-info inputonline"><i class="icon-arrow-up"></i> 1</button>
					<button class="span1 btn-info inputonline"><i class="icon-arrow-up"></i> 10</button>
				</div>
			</div>
			<div style="margin-top:10px"></div>
{{#extruder}}
			<div class="row">
				<div class="span1">Extruder {{extrudernum}}</div>
				<div class="span7">
					<div class="progress">
				  	<div id="ext{{extruderid}}_progg" class="bar bar-success" style="width: 0%;"></div>
				  	<div id="ext{{extruderid}}_progy" class="bar bar-warning" style="width: 0%;"></div>
				  	<div id="ext{{extruderid}}_progr" class="bar bar-danger" style="width: 0%;"></div>
					</div>
				</div>
				<div class="span2">
				  <span id="ext{{extruderid}}_temp">???</span>°C / <span id="ext{{extruderid}}_set">???°C</span> 
				</div>
				<button class="span2 btn btn-small btn-primary inputonline"><i class="icon-bolt"></i> Change</button>
			</div>
{{/extruder}}
		</div> {{! End control panel }}
</div></div>

{{! ================ Log =============== }}
<div class="row" style="margin-top:10px"><div class="span12">
<div class="breadcrumb" style="margin:0"><?php _("Printer Log")?> <div class="btn-group" data-toggle="buttons-checkbox">
  <button id="logcommands" type="button" class="btn btn-primary" class="span6" title="<?php _("Show commands send<br/>to the printer.")?>"><?php _("Commands") ?></button>
  <button id="logack" type="button" class="btn btn-primary" class="span2" title="<?php _("Show printer acknowledges and<br/>state change messages.")?>"><?php _("ACK") ?></button>
  <button id="logpause" type="button" class="btn btn-primary" class="span2" title="<?php _("Stops the log update so you can read<br/> the log. After restart old logs are fetched<br/>from the server if still available.")?>"><?php _("Pause log Update") ?></button>
</div></div>
<div id="logcontent" style="height:250px;overflow:auto">
<table class="table table-striped table-condensed  table-hover table-bordered">
  <tbody id="logtab">
  </tbody>
</table>
</div></div>
</div>
    </div> <!-- /container -->

<!-- Placed at the end of the document so the pages load faster -->
<script src="/jquery/jquery.min.js"></script>
<script src="/jquery/jquery.form.js"></script>
<script src="/bt/bootstrap.min.js"></script>
<script src="/bt/bootstrap-arrows.min.js"></script>
<script src="/jquery/bootbox.min.js"></script>
<script type="text/javascript">

function safe_tags(str) {
   return str.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
}
var lastoffline=false;
function checkError(ret) {
	if(ret.error=="") {
	  if(lastoffline) {
	  	lastoffline = true;
			$('#poffline').hide();
			$('.inputonline').removeAttr('disabled');
	  }
		return false;
	}
	if(ret.error="Printer offline" && lastoffline==false) {
	  lastoffline = true;
		$('#poffline').show();
			$('.inputonline').attr('disabled','');
	} 
	return true;
}
var lastlogid = 0;
var log = new Array();
function updateLog() {
	var filter = 12;
	if($('#logcommands').hasClass('active')) filter|=1;
	if($('#logack').hasClass('active')) filter|=2;
	if(!$('#logpause').hasClass('active')) { 
	 $.getJSON('/printer/response/{{slug}}?start='+lastlogid+'&filter='+filter, function(data) {
	  if(!checkError(data)) {
  	lastlogid = data.data.lastid;
  	ltab = $('#logtab');
  	changed = false;
  	$.each(data.data.lines, function(key,val) {
  	  changed = true;
    	if(log.length>100)
      	log.shift().e.remove();
    	s = '<tr id="log'+val.id+'"><td class="span2">'+val.time+'</td><td>'+safe_tags(val.text)+"</td></tr>";
    	ltab.append(s);
    	val.e = $('#log'+val.id);
    	log.push(val);
  	});
  	if(changed)
  	  $('#logcontent').scrollTop($('#logcontent')[0].scrollHeight);
  	// Update state views
  	state = data.data.state;
  	xe = $('#posx');
  	if(state.hasXHome)	xe.html("X:"+state.x.toFixed(2));
  	else 	xe.html('<span class="btn btn-mini btn-warning">X:'+state.x.toFixed(2)+"</span>");
  	ye = $('#posy');
  	if(state.hasYHome)	ye.html("Y:"+state.y.toFixed(2));
  	else 	ye.html('<span class="btn btn-mini btn-warning">Y:'+state.y.toFixed(2)+"</span>");
  	ze = $('#posz');
  	if(state.hasZHome)	ze.html("Z:"+state.z.toFixed(2));
  	else 	ze.html('<span class="btn btn-mini btn-warning">Z:'+state.z.toFixed(2)+"</span>");
  	// extruder temps
  	i=0;
  	$.each(state.extruder, function(key,val) {
  	   pre = '#ext'+i+"_";
  	   $(pre+"temp").html(val.tempRead.toFixed(2));
  	   if(val.tempSet<20) $(pre+"set").html("<?php _("Off")?>");
  	   else $(pre+"set").html(val.tempSet.toFixed(2)+"°C");
  	   // 0-70 green (23.33%), 70-260 orange (63.333%), 260-300 red (13.33%)
  	   if(val.tempRead<70) $(pre+"progg").css("width",val.tempRead*23.333/70+"%");
  	   else $(pre+"progg").css("width","23.333%");
  	   if(val.tempRead>70 && val.tempRead<260) $(pre+"progy").css("width",(val.tempRead-70)*63.333/190+"%");
  	   else $(pre+"progy").css("width",(val.tempRead<70? "0%" : "63.333%"));
  	   if(val.tempRead>260) $(pre+"progr").css("width",(val.tempRead-260)*13.333/40+"%");
  	   else $(pre+"progr").css("width","0%");
  	   i++;
  	});
  	}
	 });
	}
  setTimeout('updateLog();', 1000);
}
function sendCmd(cmd) {
  $.getJSON('/printer/send/{{slug}}?cmd='+encodeURIComponent(cmd),function(data){
  });
  return false;
}
function sendEnteredCmd(e) {
try {
	e.stopPropagation();
	sendCmd($('#command').val());$('#command').val("");
}  catch(err) {}
return false;
}

$(document).ready(function() {
	$('#startjobupload').click(function() { // Upload new job
		$('#startjobupload').button('loading');
		$('#formuploadjob').ajaxSubmit(function() {	
		  $('#startjobupload').button('reset');
	 	 $('#dialog-uploadjob').modal('hide');
		});
		return false;
	});
	updateLog();
	$('#logpause').tooltip({html:true});
	$('#logcommands').tooltip({html:true});
	$('#logack').tooltip({html:true});
	$('#sendcmd').click(sendEnteredCmd);
	$('#formcmd').submit(sendEnteredCmd);
	$('.arrow, [class^=arrow-]').bootstrapArrows();
});
</script>
</body>
</html>
