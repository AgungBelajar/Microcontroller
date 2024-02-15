import folium

# Inisialisasi peta dengan koordinat tengah
mymap = folium.Map(location=[-6.2088, 106.8456], zoom_start=10)

# Membaca data dari file teks
with open('data1.txt', 'r') as file:
    lines = file.readlines()
    prev_lat, prev_lon = None, None
    for line in lines:
        lat, lon = map(float, line.strip().split(','))
        # Menambahkan tanda pada setiap koordinat
        folium.Marker(location=[lat, lon]).add_to(mymap)
        # Menghubungkan titik dengan garis
        if prev_lat is not None and prev_lon is not None:
            folium.PolyLine(locations=[(prev_lat, prev_lon), (lat, lon)], color='blue').add_to(mymap)
        prev_lat, prev_lon = lat, lon

# Menyimpan peta ke dalam file HTML
mymap.save("map.html")
