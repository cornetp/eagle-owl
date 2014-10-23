<!doctype html>
<html>
  <head>
    <meta charset="utf-8">
    <title>Electricity live consumption</title>
    <script src="jquery-flot/jquery.js"></script>
    <script language="javascript" type="text/javascript" src="jquery-flot/jquery.flot.js"></script>
<!--    <script type="text/javascript" src="date.js"></script> -->
    <script type="text/javascript" src="datejs/build/date-fr-FR.js"></script>
  </head>
  <body>

    <h1>Electricity live consumption</h1>

    <div id="live_graph" style="width:600px;height:400px;"></div>
    <p>Update every: <input id="updateInterval" type="text" value="" style="text-align: right; width:2em"> seconds 
        ( display <a id="duration"></a> minutes )
    </p>
    
    <script type="text/javascript">
      function build_live_graph() 
      {
        var graph = $("#live_graph");
        var data = []; totalPoints = 300;
        var marks = [];
        var min = [];
        var max = [];
        function getData(addr) 
        {
          if(typeof counter == 'undefined' )
              counter = 0;
          if(typeof prev_time == 'undefined' )
            prev_time = '00:00';
          if(typeof data[addr] == 'undefined' )
            data[addr]=[];
          // init data with -1
          if(data[addr].length == 0)
          {
            for(var i = 0; i < totalPoints; ++i)
              data[addr].push(-1);
          }
          // remove the oldest value
          if(data[addr].length >= totalPoints)
            data[addr] = data[addr].slice(1);
       
          // add the new one
          function onNewData(new_data)
          {
            counter++;
            // string format: 'dd/MM/yyyy hh:mm - xxxx kW'
            var time = new_data.split(' - ')[0];
            time = time.split(' ')[1];
            var kw = new_data.split(' - ')[1];
            kw = kw.split(' ')[0];
            var v = parseFloat(kw);
            data[addr].push(v);
            if(time > prev_time)
            {
              var mark = [];
              mark['pos'] = totalPoints;
              mark['text'] = time;
              marks.push(mark);
              prev_time = time;
            }
          }

          $.ajax({
            url: "live_data_id.php?addr="+addr,
            method: 'GET',
            dataType: 'text',
            success: onNewData
          });
    
          // compute min and max
          for (key in data) {
            max[key] = Math.max.apply(null, data[key]);
            min[key] = max[key];
     
            for (var i = totalPoints-1; i >= 0; i--) 
            {
              if (data[key][i] < min[key] && data[key][i] > 0) {
                min[key] = data[key][i];
              }
            }
          }

         var index=0;
         for (key in data) {
           var off = 3+index*30;
           if(min[key]>0) {
             $('#min'+key).remove();

             graph.append('<div id="min'+key+'" class="min" style="position:absolute; right:10px; top:' + off + 'px;color:#88F;font-size:smaller">min:' + min[key] + '</div>');
           }
           if(max[key]>0 && max[key] != min[key])
           {
             $('#max'+key).remove();
             var off = 15+index*30;
             graph.append('<div id="max'+key+'" class="max" style="position:absolute; right:10px; top:' + off + 'px;color:#F88;font-size:smaller">max:' + max[key] + '</div>');
           }
           index++;
         }
          // update marks
          var num_marks = marks.length;
          for(var i = 0; i < num_marks; ++i)
            --marks[i].pos;

          if(num_marks > 0 && marks[0].pos <= 0)
            marks = marks.slice(1);
 
          // zip the generated y values with the x values
          var res = [];
          for(var i = 0; i < data[addr].length; ++i)
            res.push([i, data[addr][i]])
          return res;
        }

        // Add vertical lines when a new minute starts 
        function markings() 
        {
          var markings = [];
          for(var i = 0; i < marks.length; i++)
          {
            var mark = marks[i];
            if(marks[i].pos >= 0)
               markings.push({ color: '#888', lineWidth: 0.5, xaxis: { from: marks[i].pos, to: marks[i].pos } });
           } 
           for(key in data) { 
            if(min[key] >= 0)
              markings.push({ color: '#88F', lineWidth: 0.5, yaxis: { from: min[key], to: min[key] } });
            if(max[key] >= 0 && max != min[key])
              markings.push({ color: '#F88', lineWidth: 0.5, yaxis: { from: max[key], to: max[key] } });
           }
          
          return markings;
        }
        // Add ticks with the time value
        function xaxis_ticks()
        {
          var ticks = [];
          for(var i = 0; i < marks.length; i++)
          {
            var mark = marks[i];
            if(marks[i].pos >= 0)
            {
              var label = marks[i].text;
              // if needed, one out of two label can be positioned lower
              if(updateInterval > 3 && i%2)
                label = '<br>'+label; 
              ticks.push([marks[i].pos, label]);
            }
          }
          return ticks;
        }

        // setup control widget
        var updateInterval = 1; // in seconds
        var duration = (updateInterval*totalPoints)/60; // in minutes
        duration += '';
        document.getElementById("duration").innerHTML = duration;
        $("#updateInterval").val(updateInterval).change(function () 
        {
          var v = $(this).val();
          if (v && !isNaN(+v)) {
            updateInterval = +v;
          if (updateInterval < 1)
            updateInterval = 1;
            if (updateInterval > 10)
              updateInterval = 10;
            $(this).val("" + updateInterval);
          }
          var duration = (updateInterval*totalPoints)/60; // in minutes
          duration += '';
          document.getElementById("duration").innerHTML = duration;
        });
      
        // setup plot
        var options = {
            series: { shadowSize: 0 }, // drawing is faster without shadows
            xaxis: { ticks: xaxis_ticks },
            yaxis: { min: 0 },
            grid: { markings: markings }
        };
        var plot = $.plot(graph, [ getData(2360670),getData(2354602)], options);
      
        function update() {
            plot.setData([ getData(2360670),getData(2354602)]);
            //alert(getData(2360670)+getData(2354602));
            plot.setupGrid();
            plot.draw();
            
            setTimeout(update, updateInterval*1000);
        }
      
        update();
      }

      build_live_graph();

    </script>

  </body>
</html>
