@xdf = {}

loadAd = (data, dom_id) ->
  if data
    render(data, dom_id)

dataToObj = (data) ->
  click_url: data[0]
  show_url: data[1]
  img: data[2]
  img_desc: data[3]
  txt_title: data[4]
  txt_desc1: data[5]
  txt_desc2: data[6]
  id: data[7]
  type: data[8]
  width: data[9]
  height: data[10]

render_wenzi = (ad) ->
  [
    '<a target="_blank" style="color:#666;font-family:宋体;font-size:12px;" href="'
    ad.click_url
    '">'
    '<div style="float:left;margin:0 5px 5px 0;padding:5px;border:solid 1px #ccc;background: #eee;">'
    ad.txt_title
    '</div>'
    ad.txt_desc1
    ','
    ad.txt_desc2
    '</a>'
  ].join('')

render_img = (ad) ->
  [
    '<a style="border:0" target="_blank" href="'
    ad.click_url
    '"><img src="'
    ad.img
    '" alt="'
    ad.img_desc
    '"></a>'
  ].join('')

empty = -> ''

render_fn =
  '1': render_wenzi
  '2': render_img

render = (data, dom_id) ->
  html = []
  for i in data.set
    ad = dataToObj i
    html.push (render_fn[ad.type]||empty) ad
  $("#"+dom_id).html html.join ''

# 渲染第3种样式：悬浮广告位
render3 = (data) ->
  $("body").append('<div style="position:fixed;" id="xdf_slot_' + data.slot_id + '"><a style="border:0" href="' + data.set[0].img+ '"><img src="' + data.set[0].img+ '" alt="' + data.set[0].txt_title + '"></a></div>')
  css = {}
  if data.AdLeft
    css.left = data.AdLeft
  if data.AdTop
    css.top = data.AdTop
  if data.AdRight
    css.right = data.AdRight
  if data.AdBottom
    css.bottom = data.AdBottom
  $("#"+data.slot_id).css css

@xdf.fillSlot = (id) ->
  slot_id = 'xdf_slot_' + id
  document.write("<div id='"+slot_id+"'></div>")
  cb = (data) -> loadAd data, slot_id
  $.getJSON "http://ads.staff.xdf.cn/demo/#{id}?callback=?", cb
