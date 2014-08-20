var xdf;
xdf =(function( xdf ){
    xdf.fillSlot = function(id){
      var slot_id = 'xdf_slot_' + id;
      document.write("<div id='"+slot_id+"'></div>");
        $.get(
                "http://ads.staff.xdf.cn/demo/"+id,
            function(arr){
                xdf.loadAd(arr, slot_id);
            },
            'jsonp'
        );
    }

    //入口
    xdf.loadAd = function(adData, dom_id){
        if(adData){
            if(adData.AdType==1){
                xdf.render1(adData, dom_id);
            }
            if(adData.AdType==2){
                xdf.render2(adData, dom_id);
            }
        }
    }

    //渲染第1种样式：文字广告位
    xdf.render1 = function(adData, dom_id){
        var ele = $("#"+dom_id);
        var html = "";
        for(var i=0;i<adData.AdSet.length;i++){
            html = html+'<a style="color:#666;font-family:宋体;font-size:12px;" href="'+ adData.AdSet[i].AdClickUrl +'"><div style="float:left;margin:0 5px 5px 0;padding:5px;border:solid 1px #ccc;background: #eee;">'+ adData.AdSet[i].AdTitle + '</div></a>'
        }
        ele.html(html);
    }

    //渲染第2种样式：固定广告位
    xdf.render2 = function(adData, dom_id){
        var ele = $("#"+dom_id);
        ele.html('<a style="border:0" href="' + adData.AdSet[0].AdClickUrl + '"><img src="'+ adData.AdSet[0].AdImageUrl + '" alt="'+ adData.AdSet[0].ImgAlt +'"></a>');
    }

    //渲染第3种样式：悬浮广告位
    xdf.render3 = function(adData) {
        $("body").append('<div style="position:fixed;" id="xdf_slot_' + adData.AdSlotId + '"><a style="border:0" href="' + adData.AdSet[0].ImgLink + '"><img src="' + adData.AdSet[0].ImgSrc + '" alt="' + adData.AdSet[0].ImgAlt + '"></a></div>');
        //dom_id = 'xdf_slot_' + id;
        if(adData.AdLeft){
            $("#"+adData.AdSlotId).css({
                "left":adData.AdLeft
            });
        }
        if(adData.AdTop){
            $("#"+adData.AdSlotId).css({
                "top":adData.AdTop
            });
        }
        if(adData.AdRight){
            $("#"+adData.AdSlotId).css({
                "right":adData.AdRight
            });
        }
        if(adData.AdBottom){
            $("#"+adData.AdSlotId).css({
                "bottom":adData.AdBottom
            });
        }
    }



    return xdf;
})( xdf || {});









