// ratingMap.js — browser module, no Express needed

export const RATING_TICKERS = [
  {
    min: 9.9, max: 10.0,
    title: "Smashing Pumpkins — 1979",
    image: "musicImages/1979.jpg",
    audio: "songs/1979.m4a"
  },
  {
    min: 9.7, max: 9.89,
    title: "Daft Punk — Veridis Quo",
    image: "musicImages/discovery.jpg",
    audio: "songs/veridis_quo.m4a"
  },
  {
    min: 9.5, max: 9.69,
    title: "The Strokes — Hard to Explain",
    image: "musicImages/hardToExplain.jpg",
    audio: "songs/hard_to_explain.m4a"
  },
  {
    min: 9.3, max: 9.49,
    title: "Phoenix — 1901",
    image: "musicImages/1901.jpg",
    audio: "songs/1901.m4a"
  },
  {
    min: 9.1, max: 9.29,
    title: "New Order — Bizarre Love Triangle",
    image: "musicImages/bizzareLoveTriangel.jpg",
    audio: "songs/bizarre_love_triangle.m4a"
  },
  {
    min: 8.9, max: 9.09,
    title: "Skrillex — Cinema",
    image: "musicImages/cinema.jpg",
    audio: "songs/cinema.m4a"
  },
  {
    min: 8.7, max: 8.89,
    title: "George Clanton — Livin Loose",
    image: "musicImages/livinLoose.jpg",
    audio: "songs/livin_loose.m4a"
  },
  {
    min: 8.5, max: 8.69,
    title: "Atlas Genius — Trojans",
    image: "musicImages/trojans.jpg",
    audio: "songs/trojans.m4a"
  },
  {
    min: 8.3, max: 8.49,
    title: "Oasis — Some Might Say",
    image: "musicImages/someMightSay.jpg",
    audio: "songs/some_might_say.m4a"
  },
  {
    min: 8.1, max: 8.29,
    title: "Digitalism — Pogo",
    image: "musicImages/pogo.jpg",
    audio: "songs/pogo.m4a"
  },
  {
    min: 7.9, max: 8.09,
    title: "Goldfinger — Superman",
    image: "musicImages/superman.jpg",
    audio: "songs/superman.m4a"
  },
  {
    min: 7.7, max: 7.89,
    title: "Blues Traveler — Run Around",
    image: "musicImages/runAround.jpg",
    audio: "songs/run_around.m4a"
  },
  {
    min: 7.5, max: 7.69,
    title: "Mika — Relax, Take It Easy",
    image: "musicImages/relax.jpg",
    audio: "songs/relax_take_it_easy.m4a"
  },
  {
    min: 7.3, max: 7.49,
    title: "Walkmen — The Rat",
    image: "musicImages/theRat.jpg",
    audio: "songs/the_rat.m4a"
  },
  {
    min: 7.1, max: 7.29,
    title: "Aaron Taos — Control",
    image: "musicImages/control.jpg",
    audio: "songs/control.m4a"
  },
  {
    min: 6.9, max: 7.09,
    title: "Stone Temple Pilots — Crackerman",
    image: "musicImages/crackerman.jpg",
    audio: "songs/crackerman.m4a"
  },
  {
    min: 0.0, max: 6.89,
    title: "Hop on Overwatch",
    image: "musicImages/winston.jpg",
    audio: "songs/veridis_quo.m4a"
  }
];

export function getRatingCard(rating) {
  const match = RATING_TICKERS.find(t => rating >= t.min && rating <= t.max);
  return match || {
    title: "No match",
    image: "",
    audio: ""
  };
}